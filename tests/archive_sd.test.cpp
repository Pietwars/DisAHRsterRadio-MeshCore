#include <cassert>
#include <limits>
#define DISASTER_SD 1
#define LILYGO_TDECK 1
#define PIN_SD_CS 39
#define PIN_TFT_CS 12
#define PIN_TFT_DC 11
#define PIN_TFT_SCL 40
#define PIN_TFT_SDA 41
#define P_LORA_SCLK 40
#define P_LORA_MISO 38
#define P_LORA_MOSI 41
#define P_LORA_NSS 9
#define P_LORA_RESET 17
#define P_LORA_BUSY 13
#define P_LORA_DIO_1 45
#include "../examples/companion_radio/ArchiveSd.h"
#include "../examples/companion_radio/SdFileVerify.h"
FakeSd SD;SerialStub Serial;SPIClass tdeck_shared_spi;
size_t writeLimit=std::numeric_limits<size_t>::max(),readLimit=writeLimit;
void tdeck_shared_spi_begin(){}
void reset(){SD=FakeSd();writeLimit=readLimit=std::numeric_limits<size_t>::max();}
int main(){
 disaster::HistoryRecord record{};record.seq=1;record.len=1;
 reset();ArchiveSd good;good.begin();assert(good.ready()&&good.cardReady());
 assert(SD.frequency==800000&&!SD.format);good.append(record);
 assert(SD.files["/disaster/current.bin"].bytes.size()==sizeof(record));
 good.begin();assert(SD.beginCalls==1); // a healthy card is not remounted
 reset();SD.beginOK=false;ArchiveSd absent;absent.begin();assert(!absent.cardReady()&&!absent.ready());
 SD.beginOK=true;absent.begin();assert(absent.cardReady()&&absent.ready()&&SD.beginCalls==2);
 reset();SD.type=CARD_NONE;ArchiveSd noType;noType.begin();assert(!noType.cardReady()&&SD.endCalls==1);
 reset();SD.mkdirOK=false;ArchiveSd readOnly;readOnly.begin();assert(readOnly.cardReady()&&!readOnly.ready());
 SD.mkdirOK=true;readOnly.begin();assert(readOnly.ready()&&SD.beginCalls==1);
 reset();SD.files["/disaster"].dir=false;ArchiveSd wrongDir;wrongDir.begin();assert(wrongDir.cardReady()&&!wrongDir.ready());
 reset();ArchiveSd openFail;openFail.begin();SD.openOK=false;openFail.append(record);
 assert(openFail.cardReady()&&!openFail.ready()&&strlen(openFail.status()));
 reset();ArchiveSd shortWrite;shortWrite.begin();writeLimit=2;shortWrite.append(record);
 assert(shortWrite.cardReady()&&!shortWrite.ready());
 reset();ArchiveSd rotate;rotate.begin();SD.files["/disaster/current.bin"].bytes.resize(3);SD.renameOK=false;
 rotate.append(record);assert(rotate.cardReady()&&!rotate.ready()&&SD.files["/disaster/current.bin"].bytes.size()==3);
 reset();ArchiveSd recovered;recovered.begin();writeLimit=2;recovered.append(record);writeLimit=readLimit;
 recovered.begin();recovered.append(record);assert(recovered.ready()&&SD.files["/disaster/previous.bin"].bytes.size()==2);
 const uint8_t expected[]={1,2,3};SD.files["/disaster/config.txt"].bytes={1,2,3};
 assert(disaster::sdFileMatches(SD,"/disaster/config.txt",expected,3));
 SD.files["/disaster/config.txt"].bytes[2]=4;assert(!disaster::sdFileMatches(SD,"/disaster/config.txt",expected,3));
 SD.files["/disaster/config.txt"].bytes={1,2,3};readLimit=1;assert(!disaster::sdFileMatches(SD,"/disaster/config.txt",expected,3));
 readLimit=writeLimit;assert(!disaster::sdFileMatches(SD,"/missing",expected,3));
 assert(!disaster::sdFileMatches(SD,"/disaster",expected,0));
 puts("SD mount, retry, archive isolation and read-back tests passed");
}
