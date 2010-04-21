function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server = 0;
    var file = "";
    return(helper.DummyCall(servers[server], "Hello world"));
    for(server in servers) {
        file = helper.TryGetFileByName(servers[server], file_name);
    }
    if (file[0] == true) {
        return("Yes");
    }
    else {
        return("No");
    }
}

ReturnValue();
