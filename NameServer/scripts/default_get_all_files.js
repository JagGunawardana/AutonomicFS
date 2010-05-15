function ReturnValue() {
    var servers = helper.GetActiveNameServers();
    var server;
    var ret_val = new Array();
    ret_val = ret_val.concat(GetLocalServers());
    for(server in servers) {
        var file_list = helper.GetLocalFileListForServer(servers[server]);
        ret_val = ret_val.concat(file_list);
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

ReturnValue();
