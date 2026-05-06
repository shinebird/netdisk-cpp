let sessionID = -1;
let totalNum = 0;
let lastFileName = "";
let isControlPanelOpen = false;
let sj;
let transferring = false;

function initHandlers()
{
    document.getElementsByClassName("controlPanel")[0].addEventListener("click", ev => {ev.stopPropagation()});
    initValue("compressOptions", JSON.stringify(defaultCompressOptions));
    initValue("proxyOptions", JSON.stringify(defaultProxyOptions));
}

function switchControlPanel()
{
    if (isControlPanelOpen)
    {
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "none";
        document.getElementsByClassName("batchUploadFiles")[0].style.filter = "";
        isControlPanelOpen = false;
    }
    else
    {
        prepareCompressLevelOption();
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "inline";
        document.getElementsByClassName("batchUploadFiles")[0].style.filter = "blur(5px)";
        const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
        const compressMethod = compressOptions.selectedMethod;
        document.getElementById("compressMethod").value = compressMethod;
        document.getElementById("compressLevel").value = compressOptions[compressMethod].selectedLevel;
        prepareProxyOptions();
        isControlPanelOpen = true;
    }
}

function loginAndTransfer(uploadHost, username, password, uploadPaths, savePath)
{
    const loginInfo = {'username': username, 'password': password};
    const url = "https://" + uploadHost + "/login";
    fetch(url, {
        body: JSON.stringify(loginInfo),
        headers: {
            'Content-Type': 'application/json'
        },
        method: 'POST'
    }).then(response =>
    {
        if (response.status === 200)
        {
            return response.json();
        }
        else
        {
            throw new Error("Login failed");
        }
    }).then(data =>
    {
        if (data.loginStatus !== "success")
        {
            throw new Error("Wrong username or password");
        }
        sendTransferRequest(uploadHost, username, data.Token, uploadPaths, savePath);
    }).catch(error => {
        sj.close();
        if (error.message === 'Login failed')
        {
            window.alert("登录失败，请重试");
        }
        if (error.message === 'Wrong username or password')
        {
            window.alert("用户名或密码错误");
        }
        else
        {
            window.alert("无法登录，请检查网络");
            throw error;
        }
    }).finally(() => {
        const batchUploadFilesButton = document.getElementsByClassName("batchUploadFiles-button")[0];
        batchUploadFilesButton.onclick = () => startTransfer();
        batchUploadFilesButton.style.backgroundColor = "dodgerblue";
    })
}

function startTransfer()
{
    const batchUploadFilesButton = document.getElementsByClassName("batchUploadFiles-button")[0];
    batchUploadFilesButton.onclick = () => {};
    batchUploadFilesButton.style.backgroundColor = "#aaaaaa";
    const uploadHost = document.getElementById("uploadHost").value;
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;
    let uploadPath = document.getElementById("uploadPath").value;
    const regex = /"\s*"/g
    uploadPath = uploadPath.replaceAll(regex, "\";\"")
    let uploadPaths = uploadPath.split(";");
    const savePath = document.getElementById("savePath").value;
    let wrongInput = false;
    if (uploadHost === '')
    {
        window.alert("请输入对方主机的地址");
        wrongInput = true;
    }
    if (uploadPath === '')
    {
        window.alert("请输入需要上传的路径");
        wrongInput = true;
    }
    if (savePath === '')
    {
        window.alert("请输入对方主机上保存的路径");
        wrongInput = true;
    }
    if (username === '')
    {
        window.alert("请输入对方主机的用户名");
        wrongInput = true;
    }
    if (password === '')
    {
        window.alert("请输入对方主机的密码");
        wrongInput = true;
    }
    if (wrongInput)
    {
        batchUploadFilesButton.onclick = () => startTransfer();
        batchUploadFilesButton.style.backgroundColor = "dodgerblue";
        return;
    }
    recoverSockJS(uploadHost, username, password, uploadPaths, savePath);
}

function sendTransferRequest(uploadHost, username, token, uploadPaths, savePath)
{
    const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
    const method =  compressOptions.selectedMethod;
    const level = compressOptions[method].selectedLevel;
    const info = {'host': window.btoa(encodeURIComponent(uploadHost)),
        'username': window.btoa(encodeURIComponent(username)),
        'token': window.btoa(encodeURIComponent(token)),
        'sourcePath': window.btoa(encodeURIComponent(JSON.stringify(uploadPaths))),
        'targetPath': window.btoa(encodeURIComponent(savePath)),
        'method': method,
        'level': level,
        'sessionID': sessionID};
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    if (proxyOptions.useProxy)
    {
        info.proxyHostName = proxyOptions.proxy.hostname;
        info.proxyPort = proxyOptions.proxy.port;
    }
    document.getElementsByClassName("uploadStatus")[0].style.display = "inline";
    document.getElementsByClassName("batchUploadFiles")[0].style.display = 'none';
    transferring = true;
    fetch("https://" + window.location.host + "/service/util/batchUploadFiles", {
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
        document.getElementsByClassName("uploadStatus")[0].style.display = "none";
        resetCommonInfos();
        document.getElementById("fileTotalNum").innerText = "文件总数：（正在计算）";
        document.getElementsByClassName("batchUploadFiles")[0].style.display = 'inline';
        const batchUploadFilesButton = document.getElementsByClassName("batchUploadFiles-button")[0];
        batchUploadFilesButton.onclick = () => startTransfer();
        batchUploadFilesButton.style.backgroundColor = "dodgerblue";
        transferring = false;
        sj.close();
    });
}

function stopTransfer()
{
    stopTransferInternal(sessionID, "batchUploadCompressedFiles");
}

function recoverSockJS(uploadHost, username, password, uploadPaths, savePath)
{
    // 由于未知问题，使用WebSocket时无法连接服务器，现使用SockJS代替
    try
    {
        sj = new SockJS("https://" + window.location.host + "/webSocket/batchUploadFiles");
    }
    catch (e)
    {
        const batchUploadFilesButton = document.getElementsByClassName("batchUploadFiles-button")[0];
        batchUploadFilesButton.onclick = () => startTransfer();
        batchUploadFilesButton.style.backgroundColor = "dodgerblue";
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
            loginAndTransfer(uploadHost, username, password, uploadPaths, savePath);
        }
        if (typeof (info1.fileTotalNum) === 'number')
        {
            document.getElementById("fileTotalNum").innerText = "文件总数：" + info1.fileTotalNum;
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
            recoverSockJS(uploadHost, username, password, uploadPaths, savePath);
        }
        else
        {
            console.log("SockJS disconnected");
        }
    };
}