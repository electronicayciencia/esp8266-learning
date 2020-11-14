/* EMT API configuration 
   Reference: https://mobilitylabs.emtmadrid.es/
*/
#define EMT_LOGIN_URL "https://openapi.emtmadrid.es/v1/mobilitylabs/user/login/"
#define EMT_LOGIN_CLIENT_HEADER "X-ClientId"
#define EMT_LOGIN_PASSWORD_HEADER "passKey"

#define BUS_STOP "70"
#define BUS_LINE  "1"

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

#define EMT_TOKEN_LEN 40

#define FAIL -1

#define EMT_MAX_LINE_LEN 10

typedef struct {
   int number;     // bus id number
   int time;       // time in seconds to arrive
   int distance;   // distance in meters to stop
   char line[EMT_MAX_LINE_LEN];  // line (might be alphanumeric)
} Bus;


