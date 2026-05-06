'use strict'
let fileInfos = [];
let selectedFileID = [];
let currentPath = '';
let timeOutEvent = 0;
let isControlPanelOpen = false;
let isIconPreviewPanelOpen = false;

function initHandlers()
{
    document.getElementsByClassName("controlPanel")[0].addEventListener("click", ev => {ev.stopPropagation()});
    document.getElementsByClassName("iconPreviewPanel")[0].addEventListener("click", ev => {ev.stopPropagation()});
    document.getElementById("fileUpload").addEventListener("change", ev => {uploadFile(ev)});
    initValue("descending", "false");
    initValue("sortMethod", "byName");
    initValue("compressOptions", JSON.stringify(defaultCompressOptions));
    initValue("showCheckbox", "true");
}

function setShowCheckbox()
{
    const checkboxItem = document.getElementById("useCheckbox");
    if (checkboxItem.checked)
    {
        window.localStorage.setItem("showCheckbox", "true");
    }
    else
    {
        window.localStorage.setItem("showCheckbox", "false");
    }
    removeIconPreview();
    removeFileInfo();
}

function createPaths()
{
    let pathList = [];
    for (let i = 0; i < selectedFileID.length; i++)
    {
        pathList.push(fileInfos[selectedFileID[i]].absolutePath.replaceAll("\\", "/"));
    }
    return window.btoa(encodeURIComponent(JSON.stringify(pathList)));
}


function getFileExtension(fileName)
{
    const index = fileName.lastIndexOf(".");
    if (index !== -1)
        return fileName.slice(index + 1);
    else return "";
}

function compareTo(a, b)
{
    if (a < b)
        return -1;
    if (a === b)
        return 0;
    if (a > b)
        return 1;
}

function sortByDate(a, b)
{
    if (a.isDirectory !== b.isDirectory)
    {
        if (a.isDirectory)
            return -1;
        else return 1;
    }
    else
    {
        return compareTo(a.lastModified, b.lastModified);
    }
}

function sortByName(a, b)
{
    if (a.isDirectory !== b.isDirectory)
    {
        if (a.isDirectory)
            return -1;
        else return 1;
    }
    else
    {
        if (a.fileName === '' || b.fileName === '')
            return a.absolutePath.localeCompare(b.absolutePath)
        return a.fileName.localeCompare(b.fileName);
    }
}

function sortBySize(a, b)
{
    if (a.isDirectory !== b.isDirectory)
    {
        if (a.isDirectory)
            return -1;
        else return 1;
    }
    else
    {
        return compareTo(a.fileSize, b.fileSize);
    }
}

function sortByExt(a, b)
{
    if (a.isDirectory !== b.isDirectory)
    {
        if (a.isDirectory)
            return -1;
        else return 1;
    }
    else
    {
        if (!a.isDirectory)
        {
            const extA = getFileExtension(a.fileName);
            const extB = getFileExtension(b.fileName);
            if (extA !== extB)
            {
                return extA.localeCompare(extB);
            }
        }
        return a.fileName.localeCompare(b.fileName);
    }
}

function sortFileList()
{
    const sortMethod = window.localStorage.getItem("sortMethod");
    switch (sortMethod)
    {
        case "byDate": fileInfos.sort((a, b) => sortByDate(a, b)); break;
        case "byName": fileInfos.sort((a, b) => sortByName(a, b)); break;
        case "bySize": fileInfos.sort((a, b) => sortBySize(a, b)); break;
        case "byExt": fileInfos.sort((a, b) => sortByExt(a, b)); break;
        default: console.warn("Wrong sort method, skip sorting");
    }
    if (parseBoolean(window.localStorage.getItem("descending")))
    {
        fileInfos.reverse();
    }
}

function clickFileListNode(ev)
{
    const username = window.localStorage.getItem("username");
    const Token = window.localStorage.getItem("Token");
    {
        if (selectedFileID.length !== 0)
        {
            for (let i = 0; i < selectedFileID.length; i++)
            {
                const fileId = selectedFileID[i].toString();
                const checkBoxId = fileId + "_checkbox";
                const fileItem = document.getElementById(fileId);
                fileItem.style.backgroundColor = "";
                fileItem.setAttribute("selected", "false");
                const checkbox = document.getElementById(checkBoxId);
                if (checkbox !== null)
                {
                    document.getElementById(checkBoxId).checked = false;
                }
            }
        }
        if (ev.target.getAttribute('selected') === 'false')
        {
            showFileInfo(ev);
            selectedFileID = [];
            selectedFileID.push(Number(ev.target.getAttribute("id")));
            const checkbox = document.getElementById(ev.target.getAttribute("id") + "_checkbox");
            if (checkbox !== null)
            {
                checkbox.checked = true;
            }
            if (fileInfos[selectedFileID[0]].isDirectory === true)
            {
                ev.target.style.backgroundColor = '#ffd42921';
            }
            else
            {
                ev.target.style.backgroundColor ='#D4EFFA35';
            }
            ev.target.setAttribute("selected", 'true');
            const filePath = window.btoa(encodeURIComponent(fileInfos[selectedFileID[0]].absolutePath.replaceAll("\\", "/")));
            showDownloadLink(Token, username, filePath);
            removeIconPreview();
            showIconPreview(Token, username, filePath);
            if (fileInfos[selectedFileID[0]].isDirectory === true)
            {
                showBatchDownloadLink(Token, username, filePath);
                showDownloadZipLink(Token, username, filePath);
            }
            else
            {
                showBatchDownloadZipLink(Token, username, createPaths());
            }
            return;
        }
        if (ev.target.getAttribute('selected') === 'true')
        {
            clearSelectedFiles();
            removeDownloadLinks(ev);
            removeIconPreview();
            removeFileInfo();
        }
    }
}

function removeDownloadLinks(ev)
{
    if (ev)
    {
        const checkbox = document.getElementById(ev.target.getAttribute("id") + "_checkbox");
        if (checkbox !== null)
        {
            checkbox.checked = false;
        }
        ev.target.style.backgroundColor = '';
        ev.target.setAttribute("selected", 'false');
    }
    const downloadLink = document.getElementById("downloadLink");
    downloadLink.removeAttribute("href");
    downloadLink.removeAttribute("download");
    downloadLink.removeAttribute("rel");
    const batchDownloadLink = document.getElementById("batchDownloadLink");
    batchDownloadLink.removeAttribute("href");
    batchDownloadLink.removeAttribute("download");
    batchDownloadLink.removeAttribute("rel");
    const downloadZip = document.getElementById("downloadZip");
    downloadZip.removeAttribute("href");
    downloadZip.removeAttribute("download");
    downloadZip.removeAttribute("rel");
    document.getElementsByClassName('fileOperation')[0].style.display = 'none';
    document.getElementsByClassName('fileOperation')[1].style.display = 'none';
    document.getElementsByClassName('fileOperation')[2].style.display = 'none';
}

function showDownloadLink(Token, username, filePath)
{
    const url = "https://" + window.location.host + "/service/file/download?token=" + Token + "&username=" + username + "&path=" + filePath;
    const downloadLink = document.getElementById("downloadLink");
    downloadLink.setAttribute("href", url);
    downloadLink.setAttribute("download", fileInfos[selectedFileID[0]].fileName);
    downloadLink.setAttribute("rel", 'noopener noreferrer');
    document.getElementsByClassName('fileOperation')[0].style.display = 'block';
}

function showBatchDownloadLink(Token, username, filePath)
{
    const batchDownloadLink = document.getElementById("batchDownloadLink");
    const url1 = "https://" + window.location.host + "/service/file/batchDownload?token=" + Token + "&username=" + username + "&path=" + filePath + "&domain=" + window.btoa(encodeURIComponent(document.domain));
    batchDownloadLink.setAttribute("href", url1);
    batchDownloadLink.setAttribute("download", fileInfos[selectedFileID[0]].fileName);
    batchDownloadLink.setAttribute("rel", 'noopener noreferrer');
}

function showDownloadZipLink(Token, username, filePath)
{
    try
    {
        const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
        const compressMethod = compressOptions.selectedMethod;
        const compressLevel = compressOptions[compressMethod].selectedLevel;
        const url2 = "https://" + window.location.host + "/service/file/download?token=" + Token + "&username=" + username + "&path=" + filePath + "&domain=" + window.btoa(encodeURIComponent(document.domain)) + "&useZip=true" + "&compressMethod=" + compressMethod + "&level=" + compressLevel;
        const downloadZip = document.getElementById("downloadZip");
        downloadZip.setAttribute("href", url2);
        downloadZip.setAttribute("download", fileInfos[selectedFileID[0]].fileName);
        downloadZip.setAttribute("rel", 'noopener noreferrer');
        document.getElementsByClassName('fileOperation')[1].style.display = 'block';
        document.getElementsByClassName('fileOperation')[2].style.display = 'block';
    }
    catch (e)
    {
        window.localStorage.setItem("compressOptions", JSON.stringify(defaultCompressOptions));
        document.getElementsByClassName('fileOperation')[1].style.display = 'none';
        document.getElementsByClassName('fileOperation')[2].style.display = 'none';
    }
}

function showBatchDownloadZipLink(Token, username, paths)
{
    try
    {
        const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
        const compressMethod = compressOptions.selectedMethod;
        const compressLevel = compressOptions[compressMethod].selectedLevel;
        const url2 = "https://" + window.location.host + "/service/file/batchDownload/zip?token=" + Token + "&username=" + username + "&path=" + paths + "&level=" + compressLevel + "&compressMethod=" + compressMethod;
        const downloadZip = document.getElementById("downloadZip");
        downloadZip.setAttribute("href", url2);
        downloadZip.setAttribute("download", fileInfos[selectedFileID[0]].fileName);
        downloadZip.setAttribute("rel", 'noopener noreferrer');
        document.getElementsByClassName('fileOperation')[1].style.display = 'block';
    }
    catch (e)
    {
        window.localStorage.setItem("compressOptions", JSON.stringify(defaultCompressOptions));
        document.getElementsByClassName('fileOperation')[1].style.display = 'none';
        document.getElementsByClassName('fileOperation')[2].style.display = 'none';
    }
}

function showIconPreview(Token, username, filePath)
{
    const fileIcon = document.getElementsByClassName("fileIcon")[0];
    const icon = document.createElement("img");
    icon.src = "https://" + window.location.host + "/service/preview/fileIcon?token=" + Token + "&username=" + username + "&path=" + filePath;
    const length = fileIcon.getBoundingClientRect().bottom - fileIcon.getBoundingClientRect().top;
    icon.width = length;
    icon.height = length;
    icon.onload = () => {resizeIcon(icon, length);};
    fileIcon.appendChild(icon);
}

function removeIconPreview()
{
    const fileIcon = document.getElementsByClassName("fileIcon")[0];
    for (let i = fileIcon.childNodes.length - 1; i >= 0; i--)
    {
        fileIcon.removeChild(fileIcon.childNodes[i]);
    }
}

function resizeIcon(icon, length)
{
    if (icon.naturalWidth >= icon.naturalHeight)
    {
        icon.height = length * (icon.naturalHeight / icon.naturalWidth);
        icon.width = length;
        icon.style.marginTop = (length - icon.height) / 2 + "px";
    }
    if (icon.naturalHeight > icon.naturalWidth)
    {
        icon.width = length * (icon.naturalWidth / icon.naturalHeight);
        icon.height = length;
        icon.style.marginLeft = (length - icon.width) / 2 + "px";
    }
}

function adjustElementSize()
{
    const sideBar = document.getElementsByClassName("sideBar")[0];
    const height = window.innerHeight - 90;
    sideBar.style.height = window.innerHeight - 90 + "px"
    const filePanel = document.getElementsByClassName("filePanel")[0];
    filePanel.style.height = window.innerHeight - 90 + "px"
    const totalHeight = height - 80 - height * 0.02;
    const fileList = document.getElementsByClassName("fileList")[0];
    const fileInfoPanel = document.getElementsByClassName("fileInfoPanel")[0];
    fileList.style.height = totalHeight * 0.65 + "px"
    fileInfoPanel.style.height = filePanel.getBoundingClientRect().bottom - fileList.getBoundingClientRect().bottom - 50 + "px"
    const search = document.getElementsByClassName("search")[0];
    const searchInput = document.getElementById("goToPath");
    searchInput.style.width = search.getBoundingClientRect().right - search.getBoundingClientRect().left - 42.66 * 2 - 5 + "px"
    const fileIcon = document.getElementsByClassName("fileIcon")[0];
    fileIcon.style.width = fileIcon.getBoundingClientRect().bottom - fileIcon.getBoundingClientRect().top + "px"
    const length = fileIcon.getBoundingClientRect().bottom - fileIcon.getBoundingClientRect().top;
    if (fileIcon.children.length > 0)
    {
        resizeIcon(fileIcon.children[0], length)
    }
    document.getElementsByClassName('fileOperation')[3].style.display = 'block';
}

function showFileList()
{
    const fileList = document.getElementById("fileList");
    let i
    for (i = fileList.childNodes.length - 1; i >= 0; i--)
    {
        fileList.removeChild(fileList.childNodes[i])
    }
    for (i = 0; i < fileInfos.length; i++)
    {
        const node = document.createElement("li");
        node.setAttribute("id", i.toString(10))
        node.setAttribute("class",'file')
        node.setAttribute("selected", 'false')
        node.addEventListener("click", ev => {clickFileListNode(ev)})
        node.addEventListener("dblclick", ev => {showFilesInDirectory(ev)})
        node.addEventListener("touchstart", ev => {
            timeOutEvent = setTimeout(() => {showFilesInDirectory(ev)},500);
            return false;
        })
        node.addEventListener("touchend", ev => {
            clearTimeout(timeOutEvent);
            return false;
        })
        node.addEventListener("touchmove", ev => {
            clearTimeout(timeOutEvent);
            timeOutEvent = 0;
        })
        node.innerText = fileInfos[i].fileName;
        if (fileInfos[i].isDirectory === true)
        {
            node.setAttribute("class", "directory");
        }
        if (fileInfos[i].fileName === '')
            node.innerText = fileInfos[i].absolutePath;
        else
        {
            if (fileInfos[i].isDirectory === true)
            {
                node.innerText = fileInfos[i].fileName + '\\';
            }
        }
        const useCheckbox = window.localStorage.getItem("showCheckbox");
        if ("true" === useCheckbox)
        {
            const checkbox = document.createElement('input');
            checkbox.type = "checkbox";
            checkbox.className = "checkbox"
            checkbox.id = String(i) + "_checkbox";
            checkbox.addEventListener("click", ev => addSelectedFile(ev));
            fileList.append(checkbox);
        }
        fileList.append(node);
    }
    const fileInfoList = document.getElementsByClassName("fileInfoList")[0];
    fileInfoList.childNodes[7].innerText = '当前目录：'+ (currentPath.length <= 1? "root": currentPath);
}

function addSelectedFile(ev)
{
    const username = window.localStorage.getItem("username");
    const Token = window.localStorage.getItem("Token");
    const fileId = Number(ev.target.id.slice(0, -9));
    removeDownloadLinks();
    removeIconPreview();
    removeFileInfo();
    if (ev.target.checked)
    {
        selectedFileID.push(fileId);
        const fileItem = document.getElementById(String(fileId));
        if (fileInfos[fileId].isDirectory === true)
        {
            fileItem.style.backgroundColor = '#ffd42921';
        }
        else
        {
            fileItem.style.backgroundColor ='#D4EFFA35';
        }
        showBatchDownloadZipLink(Token, username, createPaths());
    }
    else
    {
        selectedFileID.forEach((item, index, arr) => {
            if (item === fileId)
            {
                arr.splice(index, 1);
            }
        });
        const fileItem = document.getElementById(String(fileId));
        fileItem.backgroundColor = '';
        fileItem.setAttribute("selected", 'false');
        if (selectedFileID.length > 0)
        {
            showBatchDownloadZipLink(Token, username, createPaths());
        }
    }
}

function getFileInfos(path="root")
{
    console.log(path);
    const username = window.localStorage.getItem("username");
    const Token = window.localStorage.getItem("Token");
    if (username === null || Token === null)
    {
        window.alert("请先登录");
        window.location.href = "https://" + window.location.host +"/user/login";
        return;
    }
    const filePath = {'path': path};
    fetch("https://" + window.location.host + "/service/file/listFiles", {
        body: JSON.stringify(filePath),
        headers: {
            'Content-Type': 'application/json',
            'username': username,
            'Token': Token
        },
        method: 'POST'
    }).then(response => {
        console.log(response.status)
        if (response.status === 200)
        {
            return response.json();
        }
        else if (response.status === 404)
        {
            throw new Error("No such directory");
        }
        else
        {
            throw new Error();
        }
    }).then(data => {
        currentPath = path;
        fileInfos = data;
        selectedFileID = [];
        removeFileInfo();
        removeIconPreview();
        sortFileList();
        showFileList();

    }).catch(err => {
        if (err.message === "No such directory")
        {
            window.alert("找不到该文件夹");
            return;
        }
        window.alert("请先登录");
        window.location.href = "https://" + window.location.host + "/user/login";
    })
}

function showFileInfo(event)
{
    const id = event.target.getAttribute("id");
    const fileInfoList = document.getElementsByClassName("fileInfoList")[0];
    fileInfoList.childNodes[1].innerText = '文件名：' + fileInfos[id].fileName;
    if (fileInfos[id].fileName === '')
        fileInfoList.childNodes[1].innerText = '文件名：' + fileInfos[id].absolutePath;
    if (fileInfos[id].isDirectory === true)
        fileInfoList.childNodes[3].innerText = '文件大小：';
    else
        fileInfoList.childNodes[3].innerText = '文件大小：' + showFileSize(fileInfos[id].fileSize);
    const date = new Date(fileInfos[id].lastModified);
    fileInfoList.childNodes[5].innerText = '最后修改时间：' + showDate(date);
}

function removeFileInfo()
{
    const fileInfoList = document.getElementsByClassName("fileInfoList")[0];
    fileInfoList.childNodes[1].innerText = '文件名：';
    fileInfoList.childNodes[3].innerText = '文件大小：';
    fileInfoList.childNodes[5].innerText = '最后修改时间：';
}

function showDay(day)
{
    switch (day)
    {
        case 0: return "星期日";
        case 1: return "星期一";
        case 2: return "星期二";
        case 3: return "星期三";
        case 4: return "星期四";
        case 5: return "星期五";
        case 6: return "星期六";
    }
}

function showDate(date)
{
    let hour = date.getHours().toString(10);
    let minute = date.getMinutes().toString(10);
    let second = date.getSeconds().toString(10);
    if (hour.length === 1)
        hour = '0' + hour;
    if (minute.length === 1)
        minute = '0' + minute;
    if (second.length === 1)
        second = '0' + second;
    return date.getFullYear() + '/' + (date.getMonth() + 1) + "/" + date.getDate() + " " + showDay(date.getDay()) + " " + hour + ":" + minute + ":" + second;
}

function showFilesInDirectory(event)
{
    const id = event.target.getAttribute("id");
    const currentFileInfo = fileInfos[id];
    if (currentFileInfo.isDirectory === true)
    {
        getFileInfos(currentFileInfo.absolutePath);
        selectedFileID = [];
    }
}

function parseBoolean(str)
{
    if (str === 'true')
        return true;
    if (str === 'false')
        return false;
}

function uploadFile(event)
{
    if (currentPath === 'root' || currentPath === '')
        return;
    const username = window.localStorage.getItem("username");
    const Token = window.localStorage.getItem("Token");
    if (username === null || Token === null)
    {
        window.alert("请先登录");
        window.location.href = "https://" + window.location.host + "/user/login";
        return;
    }
    let index;
    const batchUpload = event.target.files.length > 1;
    for (index = 0; index < event.target.files.length; index++)
    {
        const file = event.target.files[index];
        const fileInfo = {'path': currentPath, 'confirmed': false, 'fileName': file.name, 'size': file.size};
        fetch("https://" + window.location.host + "/service/file/checkFileExists", {
            body: JSON.stringify({'path': currentPath + file.name}),
            headers: {
                'username': username,
                'Token': Token
            },
            method: 'POST'
        }).then(res => {
            if (res.status === 200)
                return res.json();
            else throw new Error();
        }).then(data => {
            console.log(data);
            if (data)
            {
                if (window.confirm("文件已存在，是否覆盖"))
                {
                    fileInfo.confirmed = true;
                    sendUploadFile(username, Token, file, fileInfo, batchUpload);
                }
            }
            else sendUploadFile(username, Token, file, fileInfo, batchUpload);
        }).catch(err => {window.alert("上传失败");});
        window.alert("上传完成");
    }
}

function sendUploadFile(username, Token, file, fileInfo, batchUpload)
{
    const xhr = new XMLHttpRequest();
    xhr.open('POST', "https://" + window.location.host + "/service/file/upload", true);
    xhr.setRequestHeader('username', username);
    xhr.setRequestHeader('Token', Token);
    fileInfo.path = window.btoa(encodeURIComponent(fileInfo.path));
    fileInfo.fileName = window.btoa(encodeURIComponent(fileInfo.fileName));
    xhr.setRequestHeader('info', JSON.stringify(fileInfo));
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4)
        {
            if (xhr.status !== 200)
            {
                window.alert("上传失败");
                fileInfo.confirmed = false;
                return;
            }
            const result = parseBoolean(xhr.responseText);
            console.log(xhr.responseText);
            if (!result)
            {
                if (window.confirm("文件已存在，是否覆盖"))
                {
                    fileInfo.confirmed = true;
                    const xhr1 = new XMLHttpRequest();
                    xhr1.open('POST', "https://" + window.location.host + "/service/file/upload", true);
                    xhr1.setRequestHeader('username', username);
                    xhr1.setRequestHeader('Token', Token);
                    xhr1.setRequestHeader('info', JSON.stringify(fileInfo));
                    xhr1.onreadystatechange = function () {
                        if (xhr1.readyState === 4)
                        {
                            if (xhr1.status !== 200)
                            {
                                window.alert("上传失败");
                                fileInfo.confirmed = false;
                                return;
                            }
                            fileInfo.confirmed = false;
                            console.log(xhr1.responseText);
                            if (parseBoolean(xhr1.responseText))
                            {
                                if (!batchUpload)
                                {
                                    window.alert("上传成功");
                                }
                                getFileInfos(currentPath);
                            }
                            else window.alert("上传失败");
                        }
                    }
                    xhr1.upload.onprogress = function (ev){
                        showStatusBar(ev);
                    }
                    xhr1.send(file.slice(0, file.size));
                }
            }
            else
            {
                if (!batchUpload)
                {
                    window.alert("上传成功");
                }
            }
            fileInfo.confirmed = false;
            getFileInfos(currentPath);
        }
    }
    xhr.upload.onprogress = function (ev){
        showStatusBar(ev);
    }
    xhr.send(file.slice(0, file.size));
}

function showStatusBar(event)
{
    if (event.lengthComputable)
    {
        const percent = 100 * event.loaded / event.total;
        document.getElementById("uploadBar").style.display = 'inline';
        document.getElementById("percent").style.display = 'inline';
        document.getElementById('bar').style.width=percent+'%';
        document.getElementById("percent").innerText = Math.floor(percent)+'%';
        if (percent === 100)
        {
            document.getElementById('bar').style.width = 0 + '%';
            document.getElementById("percent").innerText = 0 + '%';
            document.getElementById("uploadBar").style.display = 'none';
            document.getElementById("percent").style.display = 'none';
        }
    }
}

function getParentDirInfo()
{
    if (currentPath !== 'root' && currentPath !== '')
    {
        let index = currentPath.lastIndexOf('/');
        if (index === -1)
            index = currentPath.lastIndexOf('\\');
        const parent = currentPath.slice(0, index);
        getFileInfos(parent);
    }
}

function goToPath()
{
    const path = document.getElementById("goToPath").value;
    console.log(path);
    getFileInfos(path);
}

function clearSelectedFiles()
{
    for (let i = 0; i < selectedFileID.length; i++)
    {
        const fileItem = document.getElementById(String(selectedFileID[i]));
        fileItem.style.backgroundColor = '';
        fileItem.setAttribute("selected", 'false');
    }
    selectedFileID = [];
}

function switchControlPanel()
{
    if (isControlPanelOpen)
    {
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "none";
        document.getElementById("panel").style.filter = "";
        isControlPanelOpen = false;
        sortFileList();
        showFileList();
        clearSelectedFiles();
        removeDownloadLinks();
    }
    else
    {
        prepareCompressLevelOption();
        document.getElementsByClassName("controlPanelBackground")[0].style.display = "inline";
        document.getElementById("panel").style.filter = "blur(5px)";
        document.getElementById("descending").checked = parseBoolean(window.localStorage.getItem("descending"));
        document.getElementById("sortMethod").value = window.localStorage.getItem("sortMethod");
        document.getElementById("useCheckbox").checked = parseBoolean(window.localStorage.getItem("showCheckbox"));
        const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
        const compressMethod = compressOptions.selectedMethod;
        document.getElementById("compressMethod").value = compressMethod;
        document.getElementById("compressLevel").value = compressOptions[compressMethod].selectedLevel;
        isControlPanelOpen = true;
    }
}

function setSortMethod()
{
    const sortMethodInput = document.getElementById("sortMethod");
    const descendingInput = document.getElementById("descending");
    window.localStorage.setItem("sortMethod", sortMethodInput.value);
    window.localStorage.setItem("descending", descendingInput.checked? "true": "false");
}

function showIconPreviewPanel()
{
    if (isIconPreviewPanelOpen)
    {
        document.getElementsByClassName("iconPreviewPanelBackground")[0].style.display = "none";
        document.getElementById("panel").style.filter = "";
        isIconPreviewPanelOpen = false;
    }
    else
    {
        document.getElementsByClassName("iconPreviewPanelBackground")[0].style.display = "inline";
        document.getElementById("panel").style.filter = "blur(5px)";
        isIconPreviewPanelOpen = true;
        const username = window.localStorage.getItem("username");
        const Token = window.localStorage.getItem("Token");
        const link = document.createElement("a");
        const icon = document.createElement("img");
        const filePath = window.btoa(encodeURIComponent(fileInfos[selectedFileID[0]].absolutePath.replaceAll("\\", "/")));
        const url = "https://" + window.location.host + "/service/preview/fileIcon?token=" + Token + "&username=" + username + "&path=" + filePath + "&largeIcon=true";
        icon.src = url;
        link.href = url;
        link.target = "_blank";
        const iconPreviewPanel = document.getElementsByClassName("iconPreviewPanel")[0];
        iconPreviewPanel.style.height = iconPreviewPanel.getBoundingClientRect().right - iconPreviewPanel.getBoundingClientRect().left + 'px';
        const length = iconPreviewPanel.getBoundingClientRect().bottom - iconPreviewPanel.getBoundingClientRect().top - 120;
        for (let i = iconPreviewPanel.childNodes.length - 1; i >= 2; i--)
        {
            iconPreviewPanel.removeChild(iconPreviewPanel.childNodes[i]);
        }
        icon.width = length;
        icon.height = length;
        icon.onload = () => {resizeIcon(icon, length);};
        link.appendChild(icon);
        iconPreviewPanel.appendChild(link);
    }
}