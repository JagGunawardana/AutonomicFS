function DeleteLocalFiles() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server;
    for(server in servers) {
       helper.DeleteFile(servers[server], file_name);
    }
    return(true);
}

DeleteLocalFiles();
