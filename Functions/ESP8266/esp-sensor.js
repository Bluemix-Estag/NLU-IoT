/**
  *
  * main() will be run when you invoke this action
  *
  * @param Cloud Functions actions accept a single parameter, which must be a JSON object.
  * 
  * This function needs no params
  *
  * @return The output of this action, which must be a JSON object.
  * 
  * Example of return:
  *     
  *{
  * "counter": 2473,
  * "sensor": 900
  *} 
  *
  */
  
 
 function main(params) {
    const request = require('request');
   const username = 'xxxxxxx'; // IoT Platfom Application username (from API Key) 
   const password = 'yyyyyyyy'; // IoT Platfom Application password (from API Key)
   //Change the paths from this url 
   const url = 'https://internetofthings.ibmcloud.com/api/v0002/organizations/<Your ORG ID>/device/types/<Your device type>/devices/<Your device name>/events/sensor';

   //This will build auth header for request
   const auth = {
       user: username,
       pass: password
   };
   let qs = null
    let options = {method: 'GET', url, qs, auth};
   let promise = new Promise((resolve, reject) => {
       request(options, (error, response, body) => {
           if (!error && response.statusCode === 200) {
               let j = JSON.parse(body);
               //Iot platform return a base64 object that we have to decode (glad we have a Cloud Function to do that!)
               payload_string = new Buffer(j.payload, 'base64').toString('ascii');
               payload = JSON.parse(payload_string)
               resolve(payload.d);
           } else {
               reject({
                   error: error,
                   response: response,
                   body: body
               });
           }
       });
   });
   return promise;
        
}
