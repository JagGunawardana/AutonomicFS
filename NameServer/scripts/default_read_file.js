function ReturnValue() {
    var num = helper.GetNumberOfParameters();
    var file_name = helper.GetParameter("file_name");
    var servers = helper.GetActiveAppServers();
    var server;
    var ret_val;
    for(server in servers) {
        file = helper.TryGetFileByName(servers[server], file_name);
        if (file['ret_val'] == true) { // this is our file
            var myString = file['file'].toString();
            return(myString);
        }
    }
// file not local what next !!
    return("No file");
}

ReturnValue();
