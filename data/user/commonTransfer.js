function updateStatus(info)
{
    if (typeof (info.id) === 'number' && totalNum > info.id)
    {
        return;
    }
    if (typeof (info.currentFile) === 'string')
        document.getElementById("currentFile").innerText = "当前文件：" + info.currentFile;
    if (typeof (info.size) === 'number')
        document.getElementById("fileSize").innerText = "文件大小：" + showFileSize(info.size);
    if (lastFileName !== info.currentFile)
    {
        totalNum = info.id;
        lastFileName = info.currentFile;
    }
    document.getElementById("fileNum").innerText = "已发送个数：" + totalNum.toString(10);
    if (typeof (info.percent) === 'number')
        document.getElementById("percent").innerText = "当前文件进度：" + (info.percent * 100) + "%";
    if (typeof (info.remoteId) === 'number')
        document.getElementById("receiveNum").innerText = "已接收个数：" + info.remoteId.toString(10);
}

function updateSpeed(info)
{
    if (typeof (info.speed) === 'number')
    {
        document.getElementById("speed").innerText = "速度：" + showFileSize(info.speed) + "/s";
    }
}

function getErrorInfo(str)
{
    switch (str)
    {
        case "Unsupported archive": return "不支持的格式";
        case "Connection aborted": return "连接中断";
        case "Not connected": return "无法建立连接"
        case "Internal error": return  "内部错误";
        case "Unsupported protocol": return "不支持的协议"
        default: return str;
    }
}

function resetCommonInfos()
{
    totalNum = 0;
    document.getElementById("currentFile").innerText = "当前文件：";
    document.getElementById("fileSize").innerText = "文件大小：";
    document.getElementById("fileNum").innerText = "已传输：";
    document.getElementById("speed").innerText = "速度：";
    document.getElementById("percent").innerText = "当前文件进度：";
    lastFileName = '';
}

function setUseProxy()
{
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    proxyOptions.useProxy = document.getElementById("useProxy").checked;
    window.localStorage.setItem("proxyOptions", JSON.stringify(proxyOptions));
}

function setProxyHostName()
{
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    proxyOptions.proxy.hostname = document.getElementById("proxyHostName").value.trim();
    window.localStorage.setItem("proxyOptions", JSON.stringify(proxyOptions));
}

function setProxyPort()
{
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    try
    {
        const port = Number(document.getElementById("proxyPort").value.trim());
        if (!(port >= 0 && port <= 65535))
        {
            throw new RangeError("Invalid port");
        }
        proxyOptions.proxy.port = port;
        window.localStorage.setItem("proxyOptions", JSON.stringify(proxyOptions));
    }
    catch (e)
    {
        console.error(e);
        window.alert("请输入0~65535之间的数字");
        proxyOptions.proxy.port = -1;
        window.localStorage.setItem("proxyOptions", JSON.stringify(proxyOptions));
    }
}

function prepareProxyOptions()
{
    const proxyOptions = JSON.parse(window.localStorage.getItem("proxyOptions"));
    document.getElementById("useProxy").checked = proxyOptions.useProxy;
    document.getElementById("proxyHostName").value = proxyOptions.proxy.hostname;
    if (proxyOptions.proxy.port !== -1)
    {
        document.getElementById("proxyPort").value = String(proxyOptions.proxy.port);
    }
    else
    {
        document.getElementById("proxyPort").value = "";
    }
}

function stopTransferInternal(sessionId, functionName)
{
    const info = {'sessionID': sessionId, 'type': functionName};
    fetch("https://" + window.location.host + "/service/util/stopTransfer", {
        body: JSON.stringify(info),
        method: 'POST'
    }).then(res => {
        if (res.status === 200)
            return res.json();
        else throw new Error();
    }).then(data => {
        if (data.status === 'succeed')
        {
            window.alert("成功停止传输");
        }
        else window.alert("停止传输失败：" + getErrorInfo(data.reason));
    }).catch(err => {
        window.alert("停止传输失败");
        console.error(err);
    });
}

function handleTransferResponse(data)
{
    if (data.status === 'succeed')
    {
        window.alert("传输成功");
    }
    else if (data.status === 'fail')
    {
        window.alert("传输失败：" + getErrorInfo(data.reason));
    }
    else if (data.status === 'running');
    else if (data.status === 'stop succeed');
    else window.alert("传输失败");
}