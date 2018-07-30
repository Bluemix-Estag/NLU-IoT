var Client = require('ibmiotf');
const Omega2Gpio = require('omega2-gpio'),
gpio = new Omega2Gpio();

var config = {
        "org":process.env.IOT_ORG,
        "id": process.env.IOT_ID,
        "domain":"internetofthings.ibmcloud.com",
        "type": process.env.IOT_DEVICE_TYPE,
        "auth-method": process.env.IOT_AUTH_KEY,
        "auth-token": process.env.IOT_AUTH_TOKEN
};

var outputA = gpio.pin(1);
var outputB = gpio.pin(11);
var outputC = gpio.pin(2);
var outputD = gpio.pin(3);

var deviceClient = new Client.IotfDevice(config);
deviceClient.connect();
deviceClient.on('connect', function(){
        deviceClient.publish('Begin', 'json', '{"msg":"CONECTADO", "count":0}');
        console.log('conectado');
});

deviceClient.on('command', function(command, format, payload, topic){
        console.log(command);
        if(command == 'cmd'){
                deviceClient.publish('Begin', 'json', '{"msg":"CMD RECEBIDO", "count":1}');
        }
        switch (command){
                case 'forward':
                        deviceClient.publish('Begin', 'json', '{"msg":"FORWARD", "count":2}');
                        outputA.set(1);
                        outputB.set(0);
			outputC.set(0);                                                     
                        outputD.set(0);                                                     
                        break;

                case 'right':                                                               
                        deviceClient.publish('Begin', 'json', '{"msg":"RIGHT", "count":3}');  
                        outputA.set(0);                                                       
                        outputB.set(1);                                                    
                        outputC.set(0);                                                    
                        outputD.set(0);                                                    
                        break;
                        
                case 'left':                                                               
                        deviceClient.publish('Begin', 'json', '{"msg":"LEFT", "count":4}'); 
                        outputA.set(0);                                                     
                        outputB.set(0);                                                       
                        outputC.set(1);                                                       
                        outputD.set(0);                                                                
                        break;

                case 'back':                                                               
                        deviceClient.publish('Begin', 'json', '{"msg":"BACK", "count":5}');
                        outputA.set(0);                                                    
                        outputB.set(0);                                                     
                        outputC.set(0);                                                     
                        outputD.set(1);
                        break;

                default:
			outputA.set(0);                                                    
                        outputB.set(0);                                                    
                        outputC.set(0);                                                    
                        outputD.set(0);                                                    
                        deviceClient.publish('Begin', 'json', '{"msg":"NAO ENTENDI", "count":9}');
                        break;                                                              
        }                                                                                   
});                                                                                           
                                                                                              
deviceClient.on("error", function(err){                                                           
        console.log("Error: "+err);                                                               
});
