#pragma once
#include <stdio.h>
#include <math.h>
namespace disaster {
inline void sosLocation(bool share,bool fresh,double lat,double lon,char *out,size_t size){
 if(!share)snprintf(out,size,"Standort nicht freigegeben");
 else if(!fresh||!isfinite(lat)||!isfinite(lon)||lat < -90||lat > 90||lon < -180||lon > 180)snprintf(out,size,"Kein frischer GPS-Fix");
 else snprintf(out,size,"GPS %.4f,%.4f",lat,lon);
}
}
