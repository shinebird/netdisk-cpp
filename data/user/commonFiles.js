const sizeOfKB = 1024;
const sizeOfMB = sizeOfKB * 1024;
const sizeOfGB = sizeOfMB * 1024;

function showFileSize(size)
{
    if (size > sizeOfGB)
        return size / sizeOfGB + "GB";
    if (size > sizeOfMB)
        return size / sizeOfMB + "MB";
    if (size > sizeOfKB)
        return size / sizeOfKB + "KB";
    return size + "Bytes";
}

function setCompressMethod()
{
    const compressMethodInput = document.getElementById("compressMethod");
    const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
    compressOptions.selectedMethod = compressMethodInput.value;
    window.localStorage.setItem("compressOptions", JSON.stringify(compressOptions));
    prepareCompressLevelOption();
    document.getElementById("compressLevel").value = compressOptions[compressOptions.selectedMethod].selectedLevel;
}

function prepareCompressLevelOption()
{
    const compressLevelInput = document.getElementById("compressLevel");
    let i;
    for (i = compressLevelInput.childNodes.length - 1; i >= 0; i--)
    {
        compressLevelInput.removeChild(compressLevelInput.childNodes[i]);
    }
    const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
    try
    {
        const minLevel = compressOptions[compressOptions.selectedMethod].minLevel;
        const maxLevel = compressOptions[compressOptions.selectedMethod].maxLevel;
        for (i = minLevel; i <= maxLevel; i++)
        {
            const level = document.createElement("option");
            level.value = i.toString();
            level.innerText = i.toString();
            compressLevelInput.appendChild(level);
        }
    }
    catch (e)
    {
        let tempOption = JSON.parse(JSON.stringify(defaultCompressOptions));
        tempOption.selectedMethod = compressOptions.selectedMethod;
        window.localStorage.setItem("compressOptions", JSON.stringify(tempOption));
        prepareCompressLevelOption();
    }
}

function setCompressLevel()
{
    const compressOptions = JSON.parse(window.localStorage.getItem("compressOptions"));
    compressOptions[compressOptions.selectedMethod].selectedLevel = Number(document.getElementById("compressLevel").value);
    window.localStorage.setItem("compressOptions", JSON.stringify(compressOptions));
}