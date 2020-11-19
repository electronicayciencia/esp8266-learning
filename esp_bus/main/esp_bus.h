/* EMT API configuration 
   Reference: https://mobilitylabs.emtmadrid.es/
*/
#define EMT_LOGIN_URL "https://openapi.emtmadrid.es/v1/mobilitylabs/user/login/"
#define EMT_LOGIN_CLIENT_HEADER "X-ClientId"
#define EMT_LOGIN_PASSWORD_HEADER "passKey"

#define EMT_TOKEN_LEN 40

#define EMT_CODE_OK  0
#define EMT_CODE_LOGIN_OK  1

#define EMT_TOKEN_HEADER "accessToken"
#define EMT_ARRIVES_URL "https://openapi.emtmadrid.es/v2/transport/busemtmad/stops/"BUS_STOP"/arrives/"BUS_LINE"/"
#define EMT_ARRIVES_BODY \
  "{" \
  "\"cultureInfo\":\"ES\"," \
  "\"Text_StopRequired_YN\":\"N\"," \
  "\"Text_EstimationsRequired_YN\":\"Y\"," \
  "\"Text_IncidencesRequired_YN\":\"N\"," \
  "\"DateTime_Referenced_Incidencies_YYYYMMDD\":\"20201111\"" \
  "}"

#define MAX_BUSES 2

#define I2C_SCL_IO 0        
#define I2C_SDA_IO 2 
#define BEEPER_IO  3
#define BEEP_FREQ  1000
#define BEEP_MS    100
