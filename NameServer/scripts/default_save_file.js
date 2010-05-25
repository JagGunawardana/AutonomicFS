
function SaveLocalFile() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var file_content = helper.GetParameter("file_content");
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = false;
    for(server in servers) {
        if (helper.SaveFile(servers[server], file_name, file_content) == true) {
            ret_val = true;
        }
    }
    return(ret_val);
}

function GetLocalServers() {
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = new Array();
    for(server in servers) {
        var file_list = helper.GetAllLocalFilesUnderMgt(servers[server]);
        ret_val = ret_val.concat(file_list);
    }
    return(ret_val);
}

function GetRemoteServer(server_ip) {
    var file_list = helper.GetLocalFileListForServer(server_ip);
    return(file_list);
}

function TryLocal() {
// If this file is on our machine then just save the local files
    var this_file_name = helper.GetParameter("file_name");
    var this_file_content = helper.GetParameter("file_content");
    var local_files = GetLocalServers();
    var current_file;
    var ret_val=false;
    for(current_file in local_files) {
        var current_file_name = local_files[current_file][1];
        if (this_file_name==current_file_name) {
            return(SaveLocalFile());
        }
    }
    return(ret_val);
}

function TryRemote() {
// Not a local file need to find it or save it as a new file
    // Get the files that are on all the other servers
    var this_file_name = helper.GetParameter("file_name");
    var this_file_content = helper.GetParameter("file_content");
    var All_Servers = Array();
    var remote_servers = helper.GetActiveNameServers();
    var current_server;
    var server;
    for(current_server in remote_servers) {
        All_Servers[remote_servers[current_server]] = GetRemoteServer(remote_servers[current_server]); 
    }
    for(server in All_Servers) {
        current_server = All_Servers[server];
        var remote_file;
        for(remote_file in current_server) {
            var remote_file_name=current_server[remote_file][1];
            var remote_ip=current_server[remote_file][3];
            if (this_file_name==remote_file_name) {
                return(helper.SaveRemoteFile(remote_ip, remote_file_name, this_file_content, false));
            }
        }
    }
    return(SaveNewFile());
}

function SaveNewFile() {
    var file_name = helper.GetParameter("file_name");
    var file_content = helper.GetParameter("file_content");
    // Decide on an application server (least load) and save the file there
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = false;
    // Server with lowest load is?
    server = helper.GetIdleAppServer();
    if (!server["success"]) {
        return(false);
    }
    if (helper.SaveNewFile(server, file_name, file_content) == true) {
            helper.LogMessage("Server "+server["name"]+"saving new file: "+file_name);
            ret_val = true;
    }
    return(ret_val);
}

if (!TryLocal()) {
    TryRemote();
}
