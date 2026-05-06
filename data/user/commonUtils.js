function initValue(name, defaultValue)
{
    const value = window.localStorage.getItem(name);
    if (value == null)
    {
        window.localStorage.setItem(name, defaultValue);
    }
}