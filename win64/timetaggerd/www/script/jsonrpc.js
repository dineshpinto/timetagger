function JSONRPCProxy(url, methodNames) {   
	  this.url = url;       
	  this.counter = 0; 
	  // Dynamically implement the given methods on the object
	  for(var i = 0; i < methodNames.length; i++) {         
	    var name = methodNames[i];    
	    var implement = function(name) {      
	      JSONRPCProxy.prototype[name] = function(cb,params) { this.call(name, cb, params); }    
	    }(name);    
	  }
}

JSONRPCProxy.prototype.call = function(methodName, cb, params) {       
  var req = {jsonrpc:'2.0', method: methodName, id:this.newID()}; 
  if(params != null) { 
    req.params = params; 
  }
  var ajax=$.ajax({
    url: this.url, 
    data: JSON.stringify(req), 
    type: "POST",
    contentType: "application/json", 
    success: function(rpcRes) { 
      if(cb) {
    	  if (rpcRes.error) {
    		  msg={
    				  method: methodName,
    				  params: params, 
    				  error: rpcRes.error
    		  }
    		  console.warn("RPC ERROR", msg);
    	  }
    	  cb(rpcRes.error, rpcRes.result);
      }
    }, 
    error: function(err, status, thrown) {
    	var msg={
			method: methodName,
		    params: params, 
    		response: err.responseText,
    		status: status, 
    		thrown: thrown	
    	};
    	console.warn("RPC TRANSPORT ERROR", msg)
    	throw msg;
    }   
  }); 
  return ajax;
};

JSONRPCProxy.prototype.newID = function() {   
  this.counter++; 
  return this.counter;
}
