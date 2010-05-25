function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var file_content = helper.GetParameter("file_content");
    var force_new = helper.GetParameter("force_new");
    if (force_new) {
        return(SaveNewFile(file_name, file_content));
    }
    else {
        return(SaveCurrentFile(file_name, file_content));
    }
}

function SaveNewFile(file_name, file_content) {
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

function SaveCurrentFile(file_name, file_content) {
    // Save the file to any application server that currently has it.
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = false;
    for(server in servers) {
        if (helper.SaveFile(servers[server], file_name, file_content) == true) {
            helper.LogMessage("Server "+servers[server]["name"]+"saving existing file: "+file_name);
            ret_val = true;
        }
    }
    return(ret_val);
}

ReturnValue();
