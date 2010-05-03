function ReturnValue() {
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val = new Array();
    for(server in servers) {
        var file_list = helper.GetAllFilesUnderMgt(servers[server]);
        ret_val = ret_val.concat(file_list);
    }
    return(ret_val);
}

ReturnValue();
