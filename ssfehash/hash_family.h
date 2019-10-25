#pragma once

#include <tuple>
#include <random>
#include <nmmintrin.h>
#include <MurmurHash3.h>
#include "xxhash.h"

const int HASH_FAMILY_NUM = 256;
const uint64_t SEEDS[HASH_FAMILY_NUM][3] = {
    {9302095407879081661u, 11834048905900682926u, 1498844793856334136u}, {4621107964769544749u, 18417205573292345240u, 14247448184489746496u}, {13996955450661578851u, 15805658018364402073u, 13480571332388913831u}, {918471373387800522u, 3980760726797025967u, 2613397144257923670u}, {9578309346024975358u, 356460532635200243u, 17769172268594446900u}, {7366263091616725999u, 16319069222186405277u, 8550258143176780291u}, {12411139300453756994u, 16764372173666678892u, 13981014381505873954u}, {4680699253082271853u, 14772539730925119610u, 9032371896630443419u}, {5444438183600364632u, 14003948229327254027u, 9468653938653071957u}, {14834013413519077302u, 4245533009026571253u, 17542950422847917593u}, {5869633754613820409u, 7951075663399804783u, 1320638178315030342u}, {2924139373849120352u, 11923394199245705167u, 16018723561376366560u}, {5780811682143593975u, 12353963915463819518u, 4467391694148213250u}, {14050630371683362142u, 992915756727833543u, 5412587253566083134u}, {373462059408773068u, 3720136689861490118u, 16462188448137052277u}, {6058488205518803539u, 14044111070889325784u, 14316383641938229621u}, {11837849342113358202u, 8776053879636752069u, 5627393128669905061u}, {4737327643346301820u, 16268802670717916227u, 5163956957654609448u}, {4367730929217476969u, 7299752492321073100u, 9065067532632894749u}, {430355891149725192u, 7351481278775290620u, 331679272309549458u}, {17439781680367704670u, 14968121286246841426u, 14101114086360436865u}, {11100195749225607339u, 6218464515793040710u, 9061200699203503275u}, {6311940072080584270u, 12871627035632013277u, 16711078413985682502u}, {7449662557816636810u, 10057462273274097727u, 5371240502750864934u}, {2981481480051204654u, 11667182432934858490u, 5368317924147566415u}, {2946031179611707067u, 12467645595945483144u, 15042299625562548976u}, {10610725627144134862u, 17271502452007065655u, 3758237942699775018u}, {17178097650238899742u, 3207363986299969244u, 8944088031512307125u}, {14757635212129343176u, 15364743698354512404u, 12467369746771381451u}, {5121034607449766676u, 13679211155005996102u, 7214782987697606465u}, {8057171666370980663u, 9454900967359157480u, 15890992896835970154u}, {7289383445506932539u, 2290597015387827381u, 17174977342828825447u}, {17266445485036010872u, 487375097159577229u, 10346595582596697501u}, {10325316060442197177u, 17258244751343011963u, 18197046937490201872u}, {18384401990720479252u, 6780044738534709198u, 6141028118141587854u}, {17223531459914928965u, 1698207785118065299u, 11119621894731524644u}, {1255582517770362066u, 11512474146617051564u, 3514470117278141719u}, {12390843843726964658u, 9573322452508211900u, 2635737377246830563u}, {11476086374316122344u, 11048847609945708670u, 18266958604721193168u}, {12587065935551296342u, 11615782680492603951u, 7422933562345682884u}, {18069413887505190023u, 1384048252126537851u, 9356981407979003172u}, {18272696223879310166u, 12426906210376911473u, 793273770094287244u}, {3885327520637542260u, 16623608094623663489u, 8751325916927775753u}, {9776928654603015790u, 5075460178661429054u, 8440310871373945790u}, {7555384073110415331u, 16156690407934122923u, 5176171626310641144u}, {12706928584317539278u, 4213442437176171544u, 12174848640346501713u}, {8948706836689884938u, 8268806765021567290u, 3246886215243793586u}, {11553467991222026288u, 901384918151521061u, 16235833680378051870u}, {14597732689617498857u, 1446837538488658696u, 16643133115231966889u}, {7745497788625926944u, 7736403280598757607u, 5099899159861578090u}, {7895144848312798256u, 1915816878560212044u, 12861850514708064263u}, {5107482181370634228u, 16395022980872842732u, 8087474038054955939u}, {7105820599007157039u, 16063313555048538396u, 6584273020862222913u}, {11089945940731480478u, 17305991278523583242u, 6933104367756788098u}, {17696872459691096353u, 17934625779842300865u, 14027679588225133288u}, {12888500163699412774u, 1256095377848311919u, 1827926108051373609u}, {13529443974483514568u, 15703198230002372822u, 6560089037687245053u}, {14716827949532671462u, 14251695847095072901u, 1039314155548418445u}, {9214366739685396330u, 1420170675852888857u, 12837872482959559055u}, {5636679892498522978u, 3312343229399120502u, 1327476673009182454u}, {13064393494823301843u, 8764998595364357201u, 12741118854524894827u}, {8347523064094685030u, 15420800268346736886u, 13263570034375055300u}, {11182941399789527456u, 4167066851616145955u, 11742110363514801828u}, {15912202516638074930u, 17139516252164333576u, 13567919590891320327u}, {5528609187248694577u, 1235551989783893457u, 10567178189518757148u}, {2750317024047135938u, 11405228533240194128u, 4521755620256178870u}, {10370615508963706364u, 4644432794727288027u, 1411424436402059623u}, {844016484644312785u, 3057831027687285094u, 11069466285515059853u}, {13619483512318520066u, 3707196794262801759u, 16421673131273596125u}, {18277069262716393946u, 6326569733640044108u, 6181477336887272522u}, {4003293444674123105u, 5343897420706860550u, 11870490965418968971u}, {689037724010752811u, 14725115424654784225u, 10285623550432890145u}, {14619130085045905865u, 12498089638352101668u, 12491940304281877825u}, {14894950494892543870u, 1040398852741222040u, 7175693510665944715u}, {7714268709799293645u, 4110198475212600634u, 679266725693962434u}, {5308512321134332799u, 15395385147213902202u, 354519775577201126u}, {16436982447928922388u, 12565671633930383460u, 8094691344094794504u}, {7473218099563233025u, 17065363141976072852u, 627797409023896914u}, {4045379525081970207u, 3082175764049836214u, 311055938069565099u}, {18271496508579125019u, 884685245593285587u, 10810789933528972678u}, {11268200028639064901u, 7576234146777907083u, 12364414576191562818u}, {14444021052944743740u, 15297864174872506256u, 4097503292905124979u}, {1833041169059973172u, 8667721531233423690u, 16435740738598633937u}, {13108075042860684367u, 151927218328686359u, 13293699359745490806u}, {17494679998675287263u, 7546703168630361467u, 17631631814739653669u}, {4502827210001504000u, 3683110066429109734u, 1858261491034740241u}, {1419359283166893686u, 9785589198463040995u, 1215160048684568149u}, {16129084075134180396u, 15209979800011529134u, 3940401678106748163u}, {281722170013122356u, 13819875792444706338u, 13255228678695610608u}, {13644583817697451344u, 16330675656792355904u, 18290814843720040711u}, {3791238300657636624u, 4771146920679870939u, 1870101778239683866u}, {7745481402807829086u, 10887668283850379997u, 6498296179718506639u}, {15999714612567860992u, 5210581085556688232u, 9599423067221777095u}, {12551408252830938424u, 4317946943249680280u, 8750310470078814134u}, {7546092872812986612u, 17149503558504036473u, 1326876863135598420u}, {12523779263196549817u, 3235874344320912896u, 11354176887864503587u}, {15952736161412833658u, 661308441907427609u, 4923147592788669607u}, {2006889712615340172u, 6031564023169142573u, 7695706328151697788u}, {15923322452516023632u, 8746698406401138697u, 11951018437273523983u}, {1104567132100265141u, 11959349891893959020u, 5444461825505950346u}, {12781000643752945635u, 5147291525459293460u, 1223237896153024880u}, {16213180804434766499u, 14829422500141081005u, 2843600275531860859u}, {7779977585747922024u, 11043346636551633464u, 9144052644701655396u}, {10313395853359288812u, 5479284609566361571u, 12568830440074800362u}, {5617486794741509600u, 4314459008312910246u, 454555693135306200u}, {16424823524424565037u, 5495971785732264171u, 13608000694378750038u}, {10244775586926431430u, 6667194908728982060u, 1717692885911529863u}, {9200555879960609537u, 11597984745404095998u, 5061429645030742186u}, {52595437846784741u, 9893770780891571780u, 6840961374350953004u}, {7585177469370325734u, 15190748664588089542u, 16261846701593915965u}, {1558031699583152024u, 8960898870114911281u, 13928850958186278697u}, {15770894837359757949u, 6229781893546880805u, 1920783354709613961u}, {13085909239136879394u, 3522508026222529393u, 9405539685998717453u}, {12325918387242457854u, 17536790424381351351u, 15543275305505638437u}, {16053406831400625456u, 16585039897284780044u, 3415926039001734583u}, {2637056741407402759u, 2822167525979172508u, 7595680026862975790u}, {13024905238211742525u, 6698818603397824706u, 8261324843277296452u}, {9268191346315047452u, 8781442224449178352u, 4174345898054597953u}, {5488672290547390524u, 13932997415173754483u, 3805025374758468682u}, {17269310620933439181u, 17764004413782159686u, 15752656738829121991u}, {11925519698429755009u, 3574976228295707819u, 1702516700217106491u}, {16170366983471363321u, 1099378903489362838u, 10506898919999873724u}, {9605240624423183801u, 7073986067900419986u, 6960445044210907476u}, {17528940107139884479u, 8221541482359088874u, 15891729527750757961u}, {13960294774491756234u, 5323555449985579817u, 10853345772974149432u}, {16158270863079750064u, 2981400144510187724u, 17300154501325968817u}, {10528747464874270195u, 8169550800330702918u, 284854390585596749u}, {14769313073918038288u, 3741942293151232314u, 13223667925186602416u}, {4656419020268292685u, 11034458338721591733u, 16815155280205774498u}, {17297275424668057807u, 3227523460509045614u, 6859132540834826113u}, {2383924426968197201u, 4018507781314474890u, 11872741142849653989u}, {9606956568536807369u, 15145855056312415947u, 13599052491484693893u}, {18265504640078475153u, 1633236123429480545u, 7474270257278870429u}, {1566358842147931718u, 3619888781457164464u, 9762047841318332024u}, {12448833669114150505u, 17138834742898974109u, 5499901074846514863u}, {15677291272687088686u, 10346403891760204122u, 8100954686694194139u}, {12411984194983423877u, 13916770187628446161u, 424616614494519314u}, {5381438600763712950u, 16078075445850315312u, 16507994425579505927u}, {1289178197118418496u, 17591770708730733853u, 10081767260595573252u}, {2206220335383916970u, 13215278635667507408u, 12725877987124867607u}, {17364372712263570802u, 1096485915313341149u, 18434756860752993371u}, {17218725181529374861u, 13570322511225927125u, 188482660013914227u}, {9608310720443584706u, 11219057638720874887u, 13060184077140608739u}, {1906386382494691512u, 4945652057917991303u, 11545983189184622983u}, {13494587648924246913u, 3536222803184539890u, 7208084402296769500u}, {4112791148817746333u, 18030745626183512703u, 12728029645886128194u}, {283225849406675186u, 11310658663904779059u, 9606015541566894264u}, {10770875360842976845u, 9551152502668210101u, 9590758779145555365u}, {12183141247545545828u, 7297497391681723770u, 971038238630629967u}, {6868529577776994887u, 8762800387226836447u, 7021414198665878432u}, {17334960591889968541u, 14482631821697947978u, 50924482720012495u}, {6536989767972750630u, 15568411551877393770u, 3105683150378441034u}, {16350970068949518330u, 643638844999414416u, 6318855330489972891u}, {3793843714013551233u, 5895693421794749335u, 15210840538022688998u}, {18424216163237686698u, 3878795079079548714u, 4588623412252646362u}, {14014680999389028618u, 14635731522873101881u, 14821429604707613533u}, {16618653753601996561u, 656047547574185507u, 1360663162709788638u}, {14566127965041233709u, 12040972258225829079u, 1132448297494967932u}, {18080002531582336186u, 16838937502613504555u, 12132321895458506566u}, {8640064165659211665u, 1686586239708497745u, 3456799866825494879u}, {14731070585044491053u, 11190686604657481255u, 1328957789005727219u}, {15399935827179117553u, 6011947347646079539u, 11826919984372735075u}, {16659226404836827843u, 11175397460403513398u, 16926093123187761034u}, {11275442971317753276u, 14833155666511242471u, 13116848854411653500u}, {15914115745506469464u, 3144435769064369246u, 14416909008962141464u}, {12571730487438520310u, 17935616645586707829u, 7925338421708662236u}, {1174881947888518984u, 4427456072391934511u, 13293533344329695975u}, {15476157248851558923u, 16980101816831554377u, 17397252514143052516u}, {14601126123214246191u, 5619970363200956894u, 2775843626818408365u}, {12331021636201148028u, 12700566895052319623u, 6409294303871667634u}, {16326340086514749985u, 11666030403424968466u, 10845990094834075811u}, {10383884007057460047u, 3706126105603478947u, 9700921761893837631u}, {18355512847290206026u, 2608925313285011891u, 13309503718860495309u}, {17454987285017959696u, 13458156492244933764u, 5541156806374260143u}, {5957867566684138210u, 17723250305760411497u, 562605262215008151u}, {8748014641010249452u, 2730543461170100091u, 17546265560682996448u}, {3973055404067979569u, 614184310294315081u, 8841726966739037276u}, {4381122741578725285u, 8303770794732553612u, 12098267193041363795u}, {17358139767118708108u, 2130747084016569982u, 2129055919951918731u}, {15088970428851407942u, 14784102914191093974u, 16096733070663587093u}, {14154532451570624175u, 6060532649459201573u, 9149765893356278032u}, {5694109526489779526u, 13966653385302346570u, 926037397789909652u}, {13674342725615396803u, 16699586674488802896u, 9220270961988288605u}, {14511799319573751110u, 4314366768010628842u, 8506139408400358034u}, {1685924420768343536u, 13719263653726274997u, 47457388636735335u}, {18144020874357862261u, 5259749046486611559u, 11071238289513897502u}, {18024504055458093448u, 9038516378887421629u, 14371882197833993332u}, {6245518922750865412u, 6310256639642985329u, 13491422031597854497u}, {1743289886211787019u, 11305494264802933972u, 2907035364973186786u}, {16764088650749254511u, 10383351455592812511u, 4872405740111760985u}, {13992422685011711971u, 1571570406496768779u, 14882377341372019309u}, {3103786866545004715u, 4473620278151011227u, 7632707201529714421u}, {12007423521645567418u, 321091573187568632u, 15940791208076545248u}, {6576789335471240292u, 11117904208093609322u, 452836840136504785u}, {17907605662507467516u, 12185566823870457425u, 8831287583205623249u}, {10399002401006715468u, 5857904789409278197u, 8260493151771511347u}, {9690570939606664497u, 5479879031989973916u, 7855830005118349757u}, {3613705886533846055u, 9214981705924753602u, 15244087399825986369u}, {12240092841528950062u, 10447076902117531640u, 1350090380853890780u}, {3610948570980712561u, 13156270976497118900u, 14547191142935017178u}, {5756675371610015101u, 18176101115279926147u, 6480934383182499064u}, {2561347856690509081u, 6940802608268271501u, 13991800320235857334u}, {6908037724889710628u, 9168956499385089794u, 10520813276748973893u}, {10983706103072039220u, 14698664830762982667u, 1216459170998761693u}, {13652885854954379582u, 12766388476228769051u, 11579343746438522504u}, {3215433549735613605u, 2954382844411480062u, 3091235103752605015u}, {5344276015143033699u, 11750904995456177550u, 12960764778161088445u}, {17651126062287051912u, 16589970262807379750u, 4253695977302379510u}, {15749538340277261905u, 10822174596378716370u, 6389847216212302538u}, {14266541209366420544u, 5483474814476390603u, 15652341946018310833u}, {13082690802336496663u, 11701706347626731105u, 6690409594751999774u}, {7718815854853020246u, 8345118215284759322u, 10886357916189635824u}, {12184468700677057047u, 8967060959530301996u, 15138922356094501212u}, {10843949511738239550u, 17806557323604889506u, 3277916062738058954u}, {18241366440512955733u, 1741092313659663050u, 11518834748829688417u}, {15677447295799019787u, 2125316764549081064u, 16278904671252973276u}, {12172780456833604094u, 11812563282514493907u, 689346594315411006u}, {5094238273570199626u, 9808881569292254532u, 11641299021404424436u}, {2786198186367134240u, 11925339682424516445u, 1370655732163208928u}, {5150596288441449406u, 8611436584450694508u, 5404710762390297832u}, {12589176384229739198u, 3284150574760871474u, 16387188779336580590u}, {13895126869109688468u, 11739975632052619109u, 7844470504108182693u}, {11067516005677256829u, 17465944642105658872u, 1220014763253934946u}, {11035331637677623641u, 5069594052814393460u, 11452346896014152453u}, {7337808590481006447u, 14653030637645003507u, 14668853246877828624u}, {12188237561210546730u, 16346749252323997172u, 1124580871124835383u}, {9407452506407349937u, 9382583067441798584u, 16389678879747867856u}, {8132323496222901894u, 17038556306066851884u, 9823762245278513829u}, {16070080833227855912u, 5894699474982782283u, 14434444606293560499u}, {17582439735800251648u, 9744463346274001155u, 1675871831972268615u}, {17261752733085252597u, 5380937185857189713u, 9908626613284920039u}, {1225052916605516680u, 6658971434356223755u, 15275436413930473413u}, {4219632959873979804u, 17395500012340691549u, 8213277596795085636u}, {7748312399401331454u, 4073695067392107560u, 8450129984271589996u}, {2966699349985411617u, 15380478291095224634u, 4415946722283214965u}, {16064248459148069170u, 8406303124273123538u, 15591092616820158052u}, {2178822784957170823u, 9265130801029994225u, 8020477022379597260u}, {17806390114533142499u, 6495268925108525372u, 16035118844540585687u}, {13014094030612139254u, 6606841726993031653u, 4551153428979191273u}, {2685974474391839355u, 3234152722232905447u, 14155832763354965232u}, {3527713140351479495u, 16484574353467047534u, 15594399544168052467u}, {7196280071596651730u, 12937158917654263676u, 1521598128640532046u}, {10092207219444430676u, 1944157744008854806u, 14007067283975269427u}, {10332327403969840868u, 10347112603367665252u, 9569619441034741497u}, {17631918214981257327u, 13306807595291400743u, 515072072426269940u}, {3081256694696977943u, 10734732962609059751u, 1396411776829823199u}, {2602412553409384377u, 8854407263925360284u, 15647345669809253680u}, {10150010714539579863u, 8286265885611152393u, 11905638291489955618u}, {8058209679637872301u, 12512061433078867769u, 14245188470420436086u}, {3966516753768476007u, 2043719895293388490u, 5873451055802719942u}, {8539649715767330855u, 13299128067413650391u, 8116748078529939855u}, {4161386967872628927u, 4153978479706406334u, 6515190414094873578u}, {16364670980882460973u, 4524366003375433957u, 13472500586965452643u}, {4159743980824469506u, 11196831258667339176u, 13093273290847070741u}, {3493279196742819779u, 9358662302044553222u, 4105147902825195523u}, {6986088791408725160u, 6860328716851572701u, 7664989646943258981u},
};

template <typename KEY_TYPE>
class CRC32Family {
   public:
    CRC32Family() {
        // require the size of KEY_TYPE is the times of 64 bits
        assert(sizeof(KEY_TYPE) % 8 == 0);
    }
    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index,
                                                  int mod) {
        uint64_t h1 = SEEDS[hash_index][0], h2 = SEEDS[hash_index][1],
                 h3 = SEEDS[hash_index][2];
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h1 = _mm_crc32_u64(h1, t[i]);
            h2 = _mm_crc32_u64(h2, t[i]);
            h3 = _mm_crc32_u64(h3, t[i]);
        }
        h1 %= mod;
        h2 %= mod;
        h3 %= mod;
        return std::make_tuple<uint32_t, uint32_t, uint32_t>(h1, h2, h3);
    }

    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index) {
        uint64_t h1 = SEEDS[hash_index][0], h2 = SEEDS[hash_index][1],
                 h3 = SEEDS[hash_index][2];
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h1 = _mm_crc32_u64(h1, t[i]);
            h2 = _mm_crc32_u64(h2, t[i]);
            h3 = _mm_crc32_u64(h3, t[i]);
        }
        return std::make_tuple<uint32_t, uint32_t, uint32_t>(h1, h2, h3);
    }

    uint32_t hash_once(KEY_TYPE key, int mod) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h % mod;
    }

    uint32_t hash_once(KEY_TYPE key) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h;
    }
};

template <typename KEY_TYPE>
class MixFamily {
   public:
    MixFamily() {
        // require the size of KEY_TYPE is the times of 64 bits
        assert(sizeof(KEY_TYPE) % 8 == 0);
    }
    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index,
                                                  int mod) {
        uint64_t h1 = SEEDS[hash_index][0], h2 = SEEDS[hash_index][1],
                 h3 = SEEDS[hash_index][2];
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h1 ^= t[i];
            h2 = _mm_crc32_u64(h2, t[i]*3);
            h3 = _mm_crc32_u64(h3, t[i]);
            h3 += __builtin_popcount(h3);
        }
        h1 %= mod;
        h2 %= mod;
        h3 %= mod;
        return std::make_tuple<uint32_t, uint32_t, uint32_t>(h1, h2, h3);
    }

    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index) {
        uint64_t h1 = SEEDS[hash_index][0], h2 = SEEDS[hash_index][1],
                 h3 = SEEDS[hash_index][2];
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h1 ^= t[i];
            h2 = _mm_crc32_u64(h2, t[i]*3);
            h3 = _mm_crc32_u64(h3, t[i]);
            h3 += __builtin_popcount(h3);
        }
        return std::make_tuple<uint32_t, uint32_t, uint32_t>(h1, h2, h3);
    }

    uint32_t hash_once(KEY_TYPE key, int mod) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return h % mod;
    }

    uint32_t hash_once(KEY_TYPE key) {
        uint64_t h = 12191410945815747277u;
        uint64_t *t = (uint64_t *)&key;
        for (uint32_t i = 0; i < sizeof(KEY_TYPE) / 8; i++) {
            h ^= _mm_crc32_u64(h, t[i]);
        }
        return uint32_t(h);
    }
};

template <typename KEY_TYPE>
class Murmur3Family {
   public:
    Murmur3Family() {
        // require the size of KEY_TYPE is the times of 32 bits
        assert(sizeof(KEY_TYPE) % 4 == 0);
    }
    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index,
                                                  int mod) {
        uint32_t h1, h2, h3;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][0], &h1);
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][1], &h2);
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][2], &h3);
        h1 %= mod;
        h2 %= mod;
        h3 %= mod;
        return std::make_tuple(h1, h2, h3);
    }

    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index) {
        uint32_t h1, h2, h3;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][0], &h1);
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][1], &h2);
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][2], &h3);
        return std::make_tuple(h1, h2, h3);
    }

    uint32_t hash_once(KEY_TYPE key, int mod) {
        uint32_t h;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), 1445563897, &h);
        return h % mod;
    }

    uint32_t hash_once(KEY_TYPE key) {
        uint32_t h;
        MurmurHash3_x86_32((void *)&key, sizeof(KEY_TYPE), 1445563897, &h);
        return h;
    }
};

template <typename KEY_TYPE>
class XXH32Family {
   public:
    XXH32Family() {
        // require the size of KEY_TYPE is the times of 32 bits
        assert(sizeof(KEY_TYPE) % 4 == 0);
    }
    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index,
                                                  int mod) {
        uint32_t h1 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][0]);
        uint32_t h2 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][1]);
        uint32_t h3 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][2]);
        h1 %= mod;
        h2 %= mod;
        h3 %= mod;
        return std::make_tuple(h1, h2, h3);
    }

    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index) {
        uint32_t h1 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][0]);
        uint32_t h2 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][1]);
        uint32_t h3 = XXH32((void *)&key, sizeof(KEY_TYPE), SEEDS[hash_index][2]);
        return std::make_tuple(h1, h2, h3);
    }

    uint32_t hash_once(KEY_TYPE key, int mod) {
        uint32_t h = XXH32((void *)&key, sizeof(KEY_TYPE), 1445563897);
        return h % mod;
    }

    uint32_t hash_once(KEY_TYPE key) {
        uint32_t h = XXH32((void *)&key, sizeof(KEY_TYPE), 1445563897);
        return h;
    }
};


template <typename KEY_TYPE>
class FakeRandomFamily {
   public:
    std::tuple<uint32_t, uint32_t, uint32_t> hash(KEY_TYPE key, int hash_index,
                                                  int mod) {
        (void)key;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(0, 0xffffffff);

        uint64_t h1 = ((uint64_t(dist(gen)) << 32) ^ dist(gen) ^ SEEDS[hash_index][0]) % mod;
        uint64_t h2 = ((uint64_t(dist(gen)) << 32) ^ dist(gen) ^ SEEDS[hash_index][1]) % mod;
        uint64_t h3 = ((uint64_t(dist(gen)) << 32) ^ dist(gen) ^ SEEDS[hash_index][2]) % mod;
        return std::make_tuple<uint32_t, uint32_t, uint32_t>(h1, h2, h3);
    }

    uint32_t hash_once(KEY_TYPE key, int mod) {
        (void)key;
        return (uint64_t(rand()) << 32 | rand()) % mod;
    }
};
