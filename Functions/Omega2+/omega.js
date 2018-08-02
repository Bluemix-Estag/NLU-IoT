/**
 *
 * main() será executado quando você chamar essa ação
 *
 * @param As ações do Cloud Functions aceitam um único parâmetro, que deve ser um objeto JSON.
 *
 * @return A saída dessa ação, que deve ser um objeto JSON.
 *
 */
function main(params) {
    let client = require('ibmiotf');
    let appConfig = {
        "org": "xxxxx", // Sua org no IoT Platform
        "id": "yyyy", // O tipo do seu dispositivo
        "domain": "internetofthings.ibmcloud.com",
        "auth-key": "wwwwwwwwwwwwwwwwwwwww", // Seu auth-key
        "auth-token": "zzzzZZZZZzzzzzZZZZ" // Seu auth-token
    };

    let appClient = new client.IotfApplication(appConfig);
    appClient.connect();
    appClient.on("connect", function () {
        console.log("conectado");
        var myData = {
            'DelaySeconds': 1
        };
        myData = JSON.stringify(myData);

        function send(callback) {
            if (params.color == "vermelho") {
                appClient.publishDeviceCommand("omega2", "omega2-1", "forward", "json", myData);
                callback();
                return {
                    cor: 'vermelho'
                };
            } if (params.color == "azul") {
                appClient.publishDeviceCommand("omega2", "omega2-1", "right", "json", myData);
                callback();
                return {
                    cor: 'azul'
                };
            } if (params.color == "verde") {
                appClient.publishDeviceCommand("omega2", "omega2-1", "left", "json", myData);
                callback();
                return {
                    cor: 'verde'
                };
            } if (params.color == "amarelo") {
                appClient.publishDeviceCommand("omega2", "omega2-1", "back", "json", myData);
                callback();
                return {
                    cor: 'amarelo'
                };
            } if (params.color == "off") {
                appClient.publishDeviceCommand("omega2", "omega2-1", "off", "json", myData);
                callback();
                return {
                    cor: 'off'
                };
            } else {
                callback();
                return {
                    cor: 'sem esta cor'
                };
            }
        }
        
        send(function () {
            appClient.disconnect();
            console.log('Desconectado');
        });

    });

    return {
        message: params.color
    };
}
