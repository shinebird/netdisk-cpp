function login()
{
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;
    const loginInfo = {'username': username, 'password': password};
    const url = "https://" + window.location.host + "/login";
    console.log(document.domain)
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
            return response.json()
        }
        else
        {
            throw new Error("Login failed")
        }
    }).then(data =>
    {
        if (data.loginStatus !== "success")
        {
            throw new Error("Wrong username or password")
        }
        window.localStorage.setItem("username", username)
        window.localStorage.setItem("Token", data.Token)
        window.location.href = "https://" + window.location.host + "/user/files"
    }).catch(error => {
        if (error.message === 'Login failed')
        {
            window.alert("登录失败，请重试")
        }
        if (error.message === 'Wrong username or password')
        {
            window.alert("用户名或密码错误")
        }
        else throw error
    })
}

function enterLogin(event)
{
    if (event.keyCode === 13)
        login()
}