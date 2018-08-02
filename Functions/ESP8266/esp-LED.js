/**
  *
  * main() will be run when you invoke this action
  *
  * @param Cloud Functions actions accept a single parameter, which must be a JSON object.
  *
  * Example of params for this function:
  *     {
  *         "cmd": "blink",
  *          "value": 10
  *     }
  * This will make ESP8266 blink for 10 seconds
  * 
  * @return The output of this action, which must be a JSON object.
  *
  */
  
 
 function main(params) {
    let client = require('ibmiotf');
    let appConfig = {
        "org": "xxxxx", // Your ORG name from Watson IoT Platform
        "id": "yyyy", // Any application name ID
        "domain": "internetofthings.ibmcloud.com",
        "auth-key": "wwwwwwwwwwwwwwwwwwwww", // Auth-key
        "auth-token": "zzzzZZZZZzzzzzZZZZ" // Auth-token
    };
    
    let appClient = new client.IotfApplication(appConfig);
    
    myData = {
    	"d": {
    		"fields": [
        		    {
        		        "field": params.cmd,
        			    "value": params.value
        		    }
    		    ]
    	}
    }
    
    function send(callback) {
            appClient.publishDeviceCommand("Your device type", "Your device name", params.cmd, "json", myData);
            callback();
    }
    
    let promise = new Promise((resolve, reject) => {
        appClient.connect();
        appClient.on("connect", function () {
        
            myData = JSON.stringify(myData);
        
            send(function () {
                appClient.disconnect();
                resolve({"response": 200})
            });
        });
    })
    
    
	return promise;
}