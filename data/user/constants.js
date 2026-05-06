const defaultCompressOptions = {
    "zip": {
        "minLevel": 0,
        "maxLevel": 9,
        "selectedLevel": 9
    },
    "xz": {
        "minLevel": 0,
        "maxLevel": 9,
        "selectedLevel": 9
    },
    "zstd": {
        "minLevel": 0,
        "maxLevel": 22,
        "selectedLevel": 4
    },
    "tar": {
        "minLevel": 0,
        "maxLevel": 0,
        "selectedLevel": 0
    },
    "gzip": {
        "minLevel": 0,
        "maxLevel": 9,
        "selectedLevel": 9
    },
    "bzip2": {
        "minLevel": 0,
        "maxLevel": 9,
        "selectedLevel": 9
    },
    "selectedMethod": "zip"
}

const defaultProxyOptions = {
    "useProxy": false,
    "proxy": {
        "hostname": "",
        "port": -1
    }
}