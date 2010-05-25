function DeleteRemoteFiles() {
    var file_name = helper.GetParameter("file_name");
    var remote_servers = helper.GetActiveNameServers();
    var current_server;
    var server;
    for(current_server in remote_servers) {
        helper.DeleteRemoteFile(remote_servers[current_server], file_name); 
    }
    return(SaveNewFile());
}

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
DeleteRemoteFiles();
