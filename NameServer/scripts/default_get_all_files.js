function ReturnValue() {
//    return("Hello world");
//    var servers = helper.GetActiveAppServers();
//    var server;
    var file_list=['one', 'two'];
    return("Hello world");
    var f1 = [];
    f1['file_hash']='213131231';
    f1['file_name']='file_name';
    file_list.push(f1);
    return(file_list);


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
