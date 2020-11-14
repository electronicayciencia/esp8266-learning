/* EMT API configuration 
   Reference: https://mobilitylabs.emtmadrid.es/
*/
#define EMT_LOGIN_URL "https://openapi.emtmadrid.es/v1/mobilitylabs/user/login/"
#define EMT_LOGIN_CLIENT_HEADER "X-ClientId"
#define EMT_LOGIN_PASSWORD_HEADER "passKey"

#define BUS_STOP 70
#define BUS_LINE  1

#define EMT_ARRIVES_URL "https://openapi.emtmadrid.es/v2/transport/busemtmad/stops/"BUS_STOP"/arrives/"BUS_LINE"/"
#define EMT_TOKEN_HEADER "accessToken"

#define EMT_CODE_OK 0
#define EMT_CODE_LOGIN_OK 1

#define EMT_TOKEN_LEN 40
