"""
In this example we connect to the JSON-RPC interface of the TimeTagger from a python program.

 * we assume the timetagger webserver has been started on the same host (localhost)
 * we use the tinyrpc python package that allows us to represent the timetagger webserver
   as a python class and will hide all JSON-RPC details from us. 
 * The methods of this 'proxy class' are equivalent to the JSON-RPC interface

"""

#from jsonrpc import ServiceProxy
#timetagger = ServiceProxy('http://localhost:8080/json-rpc')

from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.transports.http import HttpPostClientTransport
from tinyrpc import RPCClient

rpc_client = RPCClient(
    JSONRPCProtocol(),
    HttpPostClientTransport('http://localhost:8080/json-rpc')
)

# create the proxy class
timetagger = rpc_client.get_proxy()

timetagger.getSystemInfo()
timetagger.scanDevices()

res = timetagger.getDeviceList()
dev_id = res[0]['id'] # dev_id = '12520004J3' # FPGA serial number

res = timetagger.getModuleList(device=dev_id)
mod_id = res[0]['id'] # mod_id = 'ID-000001' # module created in web application beforehand 

timetagger.getDeviceInfo(device=dev_id)
timetagger.getModuleClasses(device=dev_id)

timetagger.getDACSettings(device=dev_id) # rename to getTriggerLevels
timetagger.setDACValue(device=dev_id, channel=0, value=0.6) # rename to setTriggerLevel

timetagger.getFilter(device=dev_id)
timetagger.setFilter(device=dev_id, filter=False)

timetagger.getModuleConfig(device=dev_id, module=mod_id)

timetagger.startModule(device=dev_id, module=mod_id)
timetagger.stopModule(device=dev_id, module=mod_id)

timetagger.createModule(device=dev_id,classname='countrate',channels=255)

#     if (method=="getSystemInfo") {
#         return server->jsrpcGetSystemInfo(out, params);
# 
#     } else if (method=="scanDevices") {
#         return server->jsrpcScanDevices(out, params);
# 
#     } else if (method=="createDevice") {
#         return server->jsrpcCreateDevice(out, params);
# 
#     } else if (method=="getDeviceList") {
#         return server->jsrpcGetDeviceList(out, params);
# 
#     } else if (method=="removeDevice") {
#         int ret=server->jsrpcRemoveDevice(out, params);
#         server->write_config();
#         return ret;
# 
#     } else if (method=="getDeviceInfo") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetInfo(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="stopDevice") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcStop(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="startDevice") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcStart(out, params);
#         else return BAD_REQUEST;
# 
# 
#     } else if (method=="renameDevice") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcRename(out, params);
#             server->write_config();
#             return ret;
#         }
#         else return BAD_REQUEST;
# 
#     } else if (method=="getDACSettings") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetDACSettings(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="setDACValue") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcSetDACValue(out, params);
#             server->write_config();
#             return ret;
#         } else return BAD_REQUEST;
# 
#     } else if (method=="getFilter") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetFilter(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="setFilter") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcSetFilter(out, params);
#             server->write_config();
#             return ret;
#         } else return BAD_REQUEST;
# 
#     } else if (method=="getLineDelay") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetLineDelay(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="setLineDelay") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcSetLineDelay(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="getModuleClasses") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetClasses(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="getModuleList") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetModules(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="createModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcCreateModule(out, params);
#             server->write_config();
#             return ret;
#         } else return BAD_REQUEST;
# 
#     } else if (method=="getModuleList") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetModules(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="startModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcStartModule(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="stopModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcStopModule(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="pauseModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcPauseModule(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="resumeModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcResumeModule(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="removeModule") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcRemoveModule(out, params);
#             server->write_config();
#             return ret;
#         } else return BAD_REQUEST;
# 
#     } else if (method=="getModuleClass") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetModuleDescriptor(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="getModuleConfig") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetModuleConfig(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="setModuleConfig") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params))) {
#             int ret=bc->jsrpcSetModuleConfig(out, params);
#             server->write_config();
#             return ret;
#         } else return BAD_REQUEST;
# 
#     } else if (method=="getData") {
#         BackendControl *bc;
#         if ((bc=server->get_backend(out, params)))
#             return bc->jsrpcGetData(out, params);
#         else return BAD_REQUEST;
# 
# 
# 
#     } else if (method=="getDatasetList") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcGetDatasetList(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="createDataset") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcCreateDataset(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="removeDataset") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcRemoveDataset(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="getDataset") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcGetDataset(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="downloadDataset") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcDownloadDataset(out, params);
#         else return BAD_REQUEST;
# 
#     } else if (method=="saveDataset") {
#         Datastore *ds;
#         if ((ds=server->get_datastore(out, params)))
#             return ds->jsrpcSaveDataset(out, params);
#         else return BAD_REQUEST;
