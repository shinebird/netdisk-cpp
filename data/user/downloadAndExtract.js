let sessionID = -1;
let totalNum = 0;
let lastFileName = "";
let sj;
let isControlPanelOpen = false;
let transferring = false;

function initHandlers()
{
    document.getElementsByClassName("controlPanel")[0].addEventListener("click", ev => {ev.stopPropagation()});
    initValue("proxyOptions", JSON.stringify(defaultProxyOptions));
}

function switchControlPanel()
{
    if (isControlPanelOpen)
    {
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "none";
        document.getElementsByClassName("downloadAndExtract")[0].style.filter = "";
        isControlPanelOpen = false;
    }
    else
    {
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "inline";
        document.getElementsByClassName("downloadAndExtract")[0].style.filter = "blur(5px)";
        prepareProxyOptions();
        isControlPanelOpen = true;
    }
}

function startTransfer()
{
    const downloadAndExtractButton = document.getElementsByClassName("downloadAndExtract-button")[0];
    downloadAndExtractButton.onclick = () => {};
    downloadAndExtractButton.style.backgroundColor = "#aaaaaa"
    const downloadLink = document.getElementById("downloadLink").value;
    const path = document.getElementById("path").value;
    let wrongInput = false;
    if (downloadLink === '')
    {
        window.alert("请输入下载链接");
        wrongInput = true;
    }
    // if (!(downloadLink.match("^http://") || downloadLink.match("^file://") || downloadLink.match("^https://") || downloadLink.match("^ftp://")))
    // {
    //     window.alert("请输入正确的下载链接");
    //     wrongInput = true;
    // }
    if (path === '')
    {
        window.alert("请输入保存路径");
        wrongInput = true;
    }
    if (wrongInput)
    {
        downloadAndExtractButton.onclick = () => startTransfer();
        downloadAndExtractButton.style.backgroundColor = "dodgerblue";
        return;
    }
    recoverSockJS(downloadLink, path);
}

function sendTransferRequest(downloadLink, path)
{
    const info = {'URL': window.btoa(encodeURIComponent(downloadLink)),
        'outPath': window.btoa(encodeURIComponent(path)),
        'sessionID': sessionID};
    document.getElementsByClassName("downloadStatus")[0].style.display = "inline";
    document.getElementsByClassName("downloadAndExtract")[0].style.display = 'none';
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    if (proxyOptions.useProxy)
    {
        info.proxyHostName = proxyOptions.proxy.hostname;
        info.proxyPort = proxyOptions.proxy.port;
    }
    transferring = true;
    fetch("https://" + window.location.host + "/service/util/downloadAndExtract", {
        body: JSON.stringify(info),
        headers: {
            'Content-Type': 'application/json'
        },
        method: 'POST'
    }).then(response => {
        if (response.status === 200)
        {
            return response.json();
        }
        else
        {
            throw new Error();
        }
    }).then(data => {
        transferring = false;
        handleTransferResponse(data);
    }).catch(err => {
        transferring = false;
        console.error(err);
        sj.close();
        window.alert("传输失败");
    }).finally(() => {
        document.getElementsByClassName("downloadStatus")[0].style.display = "none";
        resetCommonInfos();
        document.getElementsByClassName("downloadAndExtract")[0].style.display = 'inline';
        const downloadAndExtractButton = document.getElementsByClassName("downloadAndExtract-button")[0];
        downloadAndExtractButton.onclick = () => startTransfer();
        downloadAndExtractButton.style.backgroundColor = "dodgerblue";
        transferring = false;
        sj.close();
    });
}

function stopTransfer()
{
    stopTransferInternal(sessionID, "transferCompressedFiles");
}

function recoverSockJS(downloadLink, path)
{
    // 由于未知问题，使用WebSocket时无法连接服务器，现使用SockJS代替
    try
    {
        sj = new SockJS("https://" + window.location.host + "/webSocket/downloadAndExtract");
    }
    catch (e)
    {
        const downloadAndExtractButton = document.getElementsByClassName("downloadAndExtract-button")[0];
        downloadAndExtractButton.onclick = () => startTransfer();
        downloadAndExtractButton.style.backgroundColor = "dodgerblue";
        window.alert("连接出错，请刷新页面");
        return;
    }
    sj.onopen = ev => {
        console.log("SockJS connected");
        if (transferring)
        {
            const infoT = {"id": sessionID};
            sj.send(JSON.stringify(infoT));
        }
    };
    sj.onmessage = ev => {
        const info1 = JSON.parse(ev.data);
        // 开始传输
        if (!transferring && typeof (info1.currentId) === 'number')
        {
            sessionID = info1.currentId;
            sendTransferRequest(downloadLink, path);
        }
        if (typeof (info1.currentFile) === 'string' && typeof (info1.size) === 'number')
        {
            updateStatus(info1);
        }
        if (typeof (info1.speed) === 'number')
        {
            updateSpeed(info1);
        }
    };
    sj.onclose = ev => {
        if (transferring)
        {
            console.log("SockJS disconnected while transferring, reconnecting...");
            recoverSockJS(downloadLink, path);
        }
        else
        {
            console.log("SockJS disconnected");
        }
    };
}