function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var file_content = helper.GetParameter("file_content");
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = false;
    for(server in servers) {
        helper.Debug("Saving file ("+file_name+"), file content "+file_content);
        if (helper.SaveFile(servers[server], file_name, file_content) == true) {
            ret_val = true;
        }
    }
    return(ret_val);
}

ReturnValue();
