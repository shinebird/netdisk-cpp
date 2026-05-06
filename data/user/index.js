function adjustElementSize()
{
    const extensions = document.getElementsByClassName("extension");
    for (let i = 0; i < extensions.length; i++)
    {
        const extension = extensions[i];
        const length = window.innerHeight * 0.2;
        extension.style.height = length + "px";
        extension.style.width = length + "px";
    }
    const extensionIcons = document.getElementsByClassName("extensionIcon");
    for (let i = 0; i < extensionIcons.length; i++)
    {
        const extensionIcon = extensionIcons[i];
        const length = window.innerHeight * 0.2 - 4;
        extensionIcon.style.height = length + "px";
        extensionIcon.style.width = length + "px";
    }
}

function setLinks()
{
    const netdisk = document.getElementById("netdisk");
    const url_netdisk = "https://" + window.location.host + "/user/files";
    netdisk.onclick = () => window.open(url_netdisk);
    const downloadAndExtract = document.getElementById("downloadAndExtract");
    const url_downloadAndExtract = "https://" + window.location.host + "/user/downloadAndExtract";
    downloadAndExtract.onclick = () => window.open(url_downloadAndExtract);
    const batchUploadFiles = document.getElementById("batchUploadFiles");
    const url_batchUploadFiles = "https://" + window.location.host + "/user/batchUploadFiles";
    batchUploadFiles.onclick = () => window.open(url_batchUploadFiles);
}