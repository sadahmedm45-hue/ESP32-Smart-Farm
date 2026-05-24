#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <BluetoothSerial.h>
#include <ESP32Servo.h>

// --- إعدادات الشبكة و الـ MQTT ---
//const char* ssid = "saad";
//const char* password = "12345678";
const char* ssid = "Verizon-SM-N960-jamal";
const char* password = "1836071836";
const char* mqtt_server = "broker.hivemq.com"; 

// --- تعريف دبابيس المخرجات ---
#define PUMP_PIN         2
#define HEATER_PIN       4
#define ALARM_LED       12
#define SERVO_PIN       13

Servo roofServo;
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
BluetoothSerial SerialBT;

// --- متغيرات النظام الحية ---
float tempValue = 24.0;
float humidityValue = 50.0;
float soilMoisture = 60.0;
float rainValue = 0.0;

bool isManualMode = true; 
bool wifiEnabled = true;
bool bluetoothEnabled = true;
bool mqttEnabled = true;

bool pumpState = false;
bool heaterState = false;
bool alarmState = false;
int roofAngle = 0;

unsigned long lastMqttReconnect = 0;
unsigned long lastMqttPub = 0;
unsigned long lastBtSend = 0; // مؤقت لإرسال بيانات البلوتوث كل ثانية
String btBuffer = "";    
// =====================================================================================
// واجهة الويب الاحترافية المتكاملة (HTML + CSS + JS) - نسخة مضغوطة ومطهرة 100%
// =====================================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>لوحة تحكم المزرعة الذكية</title>
    <style>
        :root {--bg-color:#0b132b;--card-bg:#1c2541;--primary:#4cc9f0;--success:#06d6a0;--warning:#ffd166;--danger:#ef476f;--accent-pink:#ff007f;--text-main:#f8fafc;--text-muted:#94a3b8;}
        body {font-family:'Segoe UI',Tahoma,Arial,sans-serif;background-color:var(--bg-color);color:var(--text-main);margin:0;padding:20px;min-height:100vh;display:flex;flex-direction:column;align-items:center;}
        .container {width:100%;max-width:1100px;position:relative;}
        .dev-badge-screen {background:linear-gradient(135deg,rgba(28,37,65,0.9),rgba(11,19,43,0.9));border:1px solid rgba(76,201,240,0.3);border-top:3px solid var(--accent-pink);border-bottom:3px solid var(--primary);padding:10px 20px;border-radius:12px;text-align:center;font-size:1.1rem;font-weight:700;margin-bottom:25px;color:#fff;}
        header {text-align:center;margin-bottom:30px;padding-bottom:15px;border-bottom:2px solid rgba(255,255,255,0.05);position:relative;}
        header h1 {margin:0;font-size:2.2rem;font-weight:900;color:#fff;}
        header p {margin:5px 0 0 0;color:var(--primary);font-weight:600;}
        .lang-switcher {position:absolute;top:10px;left:10px;background:linear-gradient(135deg,#6a1b9a,#8e24aa);color:#fff;border:none;padding:8px 16px;font-weight:700;border-radius:30px;cursor:pointer;box-shadow:0 4px 15px rgba(142,36,170,0.4);transition:all 0.3s ease;z-index:100;}
        html[dir="ltr"] .lang-switcher {left:auto;right:10px;}
        .main-grid {display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:20px;margin-bottom:20px;}
        .card {background:var(--card-bg);border-radius:16px;padding:20px;box-shadow:0 10px 25px rgba(0,0,0,0.3);border:1px solid rgba(255,255,255,0.03);display:flex;flex-direction:column;}
        .card h2 {margin-top:0;font-size:1.3rem;font-weight:700;color:#fff;border-bottom:1px solid rgba(255,255,255,0.08);padding-bottom:12px;margin-bottom:15px;display:flex;align-items:center;gap:10px;}
        .control-row {display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid rgba(255,255,255,0.03);}
        .control-row:last-child {border:none;}
        .btn {font-weight:700;font-size:0.9rem;padding:8px 18px;border:none;border-radius:8px;cursor:pointer;transition:all 0.3s ease;min-width:100px;}
        .btn-active {background-color:var(--success);color:#000;}
        .btn-inactive {background-color:#475569;color:#cbd5e1;}
        .btn-mode-manual {background-color:var(--primary);color:#000;}
        .btn-mode-auto {background-color:var(--accent-pink);color:#fff;}
        .stats-grid {display:grid;grid-template-columns:1fr 1fr;gap:12px;}
        .stat-box {background:rgba(0,0,0,0.2);padding:15px;border-radius:12px;text-align:center;border:1px solid rgba(255,255,255,0.02);}
        .stat-label {font-size:0.9rem;color:var(--text-muted);font-weight:600;}
        .stat-value {font-size:1.8rem;font-weight:700;margin-top:5px;}
        .terminal-screen {background-color:#050914;border-radius:12px;padding:15px;flex-grow:1;min-height:180px;display:flex;flex-direction:column;gap:10px;border:1px solid rgba(255,255,255,0.05);}
        .status-item {padding:10px 12px;border-radius:8px;font-size:0.95rem;font-weight:700;border-inline-start:5px solid transparent;}
        .item-danger {background:rgba(239,71,111,0.1);border-inline-start-color:var(--danger);color:#ffa6bc;}
        .item-success {background:rgba(6,214,160,0.1);border-inline-start-color:var(--success);color:#9bf6df;}
        .item-warning {background:rgba(255,209,102,0.1);border-inline-start-color:var(--warning);color:#ffeaa7;}
        .item-info {background:rgba(76,201,240,0.1);border-inline-start-color:var(--primary);color:#bdefff;}
        .badge {padding:4px 12px;border-radius:6px;font-weight:700;font-size:0.85rem;}
        .badge-on {background:rgba(6,214,160,0.15);color:var(--success);border:1px solid var(--success);}
        .badge-off {background:rgba(148,163,184,0.1);color:var(--text-muted);border:1px solid #475569;}
        .sliders-card {width:100%;box-sizing:border-box;}
        .sliders-grid {display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:20px;}
        .slider-group {display:flex;flex-direction:column;gap:8px;}
        .slider-group label {display:flex;justify-content:space-between;font-weight:600;color:var(--text-muted);font-size:0.9rem;}
        .slider-group label span:last-child {color:var(--primary);font-weight:700;}
        .range-slider {width:100%;-webkit-appearance:none;background:#2d3748;height:8px;border-radius:4px;outline:none;}
        .range-slider::-webkit-slider-thumb {-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:var(--primary);cursor:pointer;}
    </style>
</head>
<body>
    <button id="langBtn" class="lang-switcher" onclick="toggleLang()">English</button>
    <div class="container">
        <div id="devScreen" class="dev-badge-screen">⚙️ برمجة واعداد : م/ سعد أحمد معيض علبوهان</div>
        <header>
            <h1 id="headerTitle">🌿 لوحة الإدارة الهجينة للمزرعة الذكية</h1>
            <p id="headerSub">نظام المراقبة والأتمتة والتحكم بالصلاحيات المباشر</p>
        </header>
        <div class="main-grid">
            <div class="card">
                <h2 id="card1Title">⚙️ صلاحيات النظام والشبكات</h2>
                <div class="control-row"><span id="lblMode">وضع التحكم بالمزرعة:</span><button id="modeBtn" class="btn btn-mode-manual" onclick="toggleMode()">يدوي</button></div>
                <div class="control-row"><span id="lblWifi">خادم الويب (Wi-Fi Web Server):</span><button id="wifiBtn" class="btn btn-active" onclick="togglePerm('wifi')">نشط</button></div>
                <div class="control-row"><span id="lblBt">اتصال البلوتوث (Bluetooth):</span><button id="btBtn" class="btn btn-active" onclick="togglePerm('bt')">نشط</button></div>
                <div class="control-row"><span id="lblMqtt">بث السحابة (MQTT Client):</span><button id="mqttBtn" class="btn btn-active" onclick="togglePerm('mqtt')">نشط</button></div>
            </div>
            <div class="card">
                <h2 id="card2Title">📊 شاشة قراءات المستشعرات</h2>
                <div class="stats-grid">
                    <div class="stat-box"><div class="stat-label" id="lblTemp">درجة الحرارة</div><div class="stat-value" id="dispTemp" style="color:#ff6b6b;">-- °C</div></div>
                    <div class="stat-box"><div class="stat-label" id="lblHum">الرطوبة الجوية</div><div class="stat-value" id="dispHum" style="color:var(--primary);">-- %</div></div>
                    <div class="stat-box"><div class="stat-label" id="lblSoil">رطوبة التربة</div><div class="stat-value" id="dispSoil" style="color:var(--success);">-- %</div></div>
                    <div class="stat-box"><div class="stat-label" id="lblRain">مستوى المطر</div><div class="stat-value" id="dispRain" style="color:#a18cd1;">-- %</div></div>
                </div>
            </div>
            <div class="card">
                <h2 id="card3Title">🤖 شاشة الحالات والقرارات الذكية</h2>
                <div class="terminal-screen" id="aiScreen"><div class="status-item item-info">جاري استقبال البيانات وتحليلها...</div></div>
                <h2 id="card3Sub" style="margin-top:15px;margin-bottom:10px;font-size:1.1rem;padding-bottom:5px;">🔌 المخرجات الحالية</h2>
                <div style="display:flex;gap:10px;flex-wrap:wrap;">
                    <div id="lblOutPump"><span id="txtPumpLabel">المضخة: </span><span id="lblPump" class="badge badge-off">مطفأة</span></div>
                    <div id="lblOutHeater"><span id="txtHeaterLabel">التدفئة: </span><span id="lblHeater" class="badge badge-off">مطفأة</span></div>
                    <div id="lblOutServo"><span id="txtServoLabel">السقف: </span><span id="lblServo" class="badge badge-off">مفتوح</span></div>
                </div>
            </div>
        </div>
        <div class="card sliders-card">
            <h2 id="card4Title">🎛️ محاكاة المتغيرات البيئية (نشط في الوضع اليدوي فقط)</h2>
            <div class="sliders-grid">
                <div class="slider-group"><label><span id="lblSimTemp">درجة الحرارة:</span><span><span id="tVal">24</span>°C</span></label><input type="range" id="tSlider" class="range-slider" min="-5" max="50" value="24" oninput="updateSensor('temp',this.value)"></div>
                <div class="slider-group"><label><span id="lblSimHum">الرطوبة الجوية:</span><span><span id="hVal">50</span>%</span></label><input type="range" id="hSlider" class="range-slider" min="10" max="100" value="50" oninput="updateSensor('humid',this.value)"></div>
                <div class="slider-group"><label><span id="lblSimSoil">رطوبة التربة:</span><span><span id="sVal">60</span>%</span></label><input type="range" id="sSlider" class="range-slider" min="0" max="100" value="60" oninput="updateSensor('soil',this.value)"></div>
                <div class="slider-group"><label><span id="lblSimRain">شدة هطول المطر:</span><span><span id="rVal">0</span>%</span></label><input type="range" id="rSlider" class="range-slider" min="0" max="100" value="0" oninput="updateSensor('rain',this.value)"></div>
            </div>
        </div>
    </div>
    <script>
        let currentLang='ar';
        function updateStaticTexts(){
            const isAr=currentLang==='ar';
            document.getElementById('devScreen').innerText=isAr?"⚙️ برمجة واعداد : م/ سعد أحمد معيض علبوهان":"⚙️ Programming and Preparation :Eng./Saad Ahmed Moith Al-bohan";
            document.getElementById('headerTitle').innerText=isAr?"🌿 لوحة الإدارة الهجينة للمزرعة الذكية":"🌿 Hybrid Smart Farm Dashboard";
            document.getElementById('headerSub').innerText=isAr?"نظام المراقبة والأتمتة والتحكم بالصلاحيات المباشر":"Live Monitoring, Automation & Permissions System";
            document.getElementById('card1Title').innerHTML=isAr?"⚙️ صلاحيات النظام والشبكات":"⚙️ System & Network Permissions";
            document.getElementById('lblMode').innerText=isAr?"وضع التحكم بالمزرعة:":"Farm Control Mode:";
            document.getElementById('lblWifi').innerText=isAr?"خادم الويب (Wi-Fi Web Server):":"Wi-Fi Web Server:";
            document.getElementById('lblBt').innerText=isAr?"اتصال البلوتوث (Bluetooth):":"Bluetooth Connectivity:";
            document.getElementById('lblMqtt').innerText=isAr?"بث السحابة (MQTT Client):":"Cloud Stream (MQTT Client):";
            document.getElementById('card2Title').innerHTML=isAr?"📊 شاشة قراءات المستشعرات":"📊 Sensor Readings Screen";
            document.getElementById('lblTemp').innerText=isAr?"درجة الحرارة":"Temperature";
            document.getElementById('lblHum').innerText=isAr?"الرطوبة الجوية":"Humidity";
            document.getElementById('lblSoil').innerText=isAr?"رطوبة التربة":"Soil Moisture";
            document.getElementById('lblRain').innerText=isAr?"مستوى المطر":"Rain Level";
            document.getElementById('card3Title').innerHTML=isAr?"🤖 شاشة الحالات والقرارات الذكية":"🤖 Smart Decisions & Status Screen";
            document.getElementById('card3Sub').innerText=isAr?"🔌 المخرجات الحالية":"🔌 Current Actuators State";
            document.getElementById('txtPumpLabel').innerText=isAr?"المضخة: ":"Pump: ";
            document.getElementById('txtHeaterLabel').innerText=isAr?"التدفئة: ":"Heater: ";
            document.getElementById('txtServoLabel').innerText=isAr?"السقف: ":"Roof: ";
            document.getElementById('card4Title').innerHTML=isAr?"🎛️ محاكاة المتغيرات البيئية (نشط في الوضع اليدوي فقط)":"🎛️ Environmental Simulation (Active in Manual Mode only)";
            document.getElementById('lblSimTemp').innerText=isAr?"درجة الحرارة:":"Temperature:";
            document.getElementById('lblSimHum').innerText=isAr?"الرطوبة الجوية:":"Humidity:";
            document.getElementById('lblSimSoil').innerText=isAr?"رطوبة التربة:":"Soil Moisture:";
            document.getElementById('lblSimRain').innerText=isAr?"شدة هطول المطر:":"Rain Intensity:";
        }
        function toggleLang(){
            currentLang=(currentLang==='ar')?'en':'ar';
            document.documentElement.dir=(currentLang==='ar')?'rtl':'ltr';
            document.documentElement.lang=currentLang;
            document.getElementById('langBtn').innerText=(currentLang==='ar')?'English':'العربية';
            updateStaticTexts();
            requestData();
        }
        function requestData(){
            fetch('/readings')
            .then(res=>res.json())
            .then(data=>{
                const isAr=currentLang==='ar';
                document.getElementById('dispTemp').innerText=data.temp+" °C";
                document.getElementById('dispHum').innerText=data.humid+" %";
                document.getElementById('dispSoil').innerText=data.soil+" %";
                document.getElementById('dispRain').innerText=data.rain+" %";
                syncButton('modeBtn',data.mode,isAr?'يدوي':'Manual',isAr?'تلقائي':'Auto','btn-mode-manual','btn-mode-auto');
                syncButton('wifiBtn',data.wifi,isAr?'نشط':'Active',isAr?'محظور':'Blocked','btn-active','btn-inactive');
                syncButton('btBtn',data.bt,isAr?'نشط':'Active',isAr?'محظور':'Blocked','btn-active','btn-inactive');
                syncButton('mqttBtn',data.mqtt,isAr?'نشط':'Active',isAr?'محظور':'Blocked','btn-active','btn-inactive');
                updateBadge('lblPump',data.pump,isAr?'تعمل الآن':'Running Now',isAr?'مطفأة':'OFF');
                updateBadge('lblHeater',data.heater,isAr?'تدفئة نشطة':'Heater Active',isAr?'مطفأة':'OFF');
                let sTxt=isAr?(data.servo>90?"مغلق ("+data.servo+"°)":"مفتوح ("+data.servo+"°)"): (data.servo>90?"Closed ("+data.servo+"°)":"Open ("+data.servo+"°)");
                document.getElementById('lblServo').innerText=sTxt;
                document.getElementById('lblServo').className=data.servo>90?"badge badge-on":"badge badge-off";
                let log="";
                if(data.temp<3){log+=`<div class="status-item item-danger">${isAr?'🚨 تحذير صقيع! تم تشغيل المكيفات للتدفئة فوراُ!':'🚨 Frost Warning! Heaters turned ON immediately!'}</div>`;}
                else if(data.temp>5){log+=`<div class="status-item item-success">${isAr?'✅ زال الخطر. تم إطفاء المكيفات (الحرارة مستقرة).':'✅ Danger Cleared. Heaters turned OFF (Stable Temp).'}</div>`;}
                else{log+=`<div class="status-item item-info">${isAr?'❄️ درجة الحرارة منخفضة ونراقب الوضع بحذر.':'❄️ Low temperature, monitoring the situation carefully.'}</div>`;}
                if(data.soil<50){log+=`<div class="status-item item-warning">${isAr?'⚠️ تحذير جفاف! رطوبة التربة منخفضة، تم بدء تشغيل المضخات.':'⚠️ Drought Warning! Soil moisture low, pumps started.'}</div>`;}
                else{log+=`<div class="status-item item-success">${isAr?'🌱 متحسن. عملية انتهاء الري مستقرة والمضخات مطفأة.':'🌱 Improved. Irrigation completed successfully, pumps are OFF.'}</div>`;}
                if(data.rain==0){log+=`<div class="status-item item-info">${isAr?'☀️ حالة الطقس: المطر واقف تماماً.':'☀️ Weather State: Rain stopped completely.'}</div>`;}
                else if(data.rain>0&&data.rain<=25){log+=`<div class="status-item item-info">${isAr?'🌦️ حالة الطقس: رشة مطر خفيفة ومستقرة.':'🌦️ Weather State: Light stable drizzle.'}</div>`;}
                else if(data.rain>25&&data.rain<=65){log+=`<div class="status-item item-warning">${isAr?'🌧️ حالة الطقس: هطول مطر متوسط.':'🌧️ Weather State: Moderate rain falling.'}</div>`;}
                else{log+=`<div class="status-item item-danger">${isAr?'⛈️ حالة الطقس: هطول مطر غزير جداً! عاصفة قائمة.':'⛈️ Weather State: Very heavy rain! Ongoing storm.'}</div>`;}
                document.getElementById('aiScreen').innerHTML=log;
            }).catch(err=>console.log("Sync Error..."));
        }
        requestData();
        setInterval(requestData,1000);
        function syncButton(id,state,actTxt,inactTxt,actCls,inactCls){
            let b=document.getElementById(id);
            if(b){b.innerText=state?actTxt:inactTxt;b.className="btn "+(state?actCls:inactCls);}
        }
        function updateBadge(id,state,onTxt,offTxt){
            let b=document.getElementById(id);
            if(b){b.innerText=state?onTxt:offTxt;b.className=state?"badge badge-on":"badge badge-off";}
        }
        function updateSensor(type,val){
            document.getElementById(type[0]+'Val').innerText=val;
            fetch(`/update?type=${type}&value=${val}`);
        }
        function toggleMode(){fetch('/toggleMode');}
        function togglePerm(proto){fetch(`/togglePerm?proto=${proto}`);}
    </script>
</body>
</html>
)rawliteral";

// --- منطق الأتمتة الذكي للمتحكم ---
void executeAutomationLogic() {
    if (!isManualMode) {
        tempValue = 26.0;
        humidityValue = 55.0;
        soilMoisture = 55.0;
        rainValue = 10.0;
    }

    if (soilMoisture < 50.0) {
        pumpState = true;
    } else {
        pumpState = false;
    }

    if (tempValue < 3.0) {
        heaterState = true;
        alarmState = true; 
    } else if (tempValue > 5.0) {
        heaterState = false; 
        alarmState = false; 
    }

    digitalWrite(PUMP_PIN, pumpState);
    digitalWrite(HEATER_PIN, heaterState);
    digitalWrite(ALARM_LED, alarmState);
    
    if (rainValue > 65.0) roofAngle = 180; 
    else roofAngle = 0;
    roofServo.write(roofAngle);
}

// --- الإعداد الأساسي (Setup) ---
void setup() {
    Serial.begin(115200);
    
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(ALARM_LED, OUTPUT);
    roofServo.attach(SERVO_PIN);

    // بدء تشغيل الواي فاي أولاً بشكل منفرد لحجز موارد الراديو
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 40) { // زيادة مهلة الانتظار لـ 20 ثانية
        delay(500); 
        Serial.print("."); 
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n==================================");
        Serial.print("Connected successfully! IP: ");
        Serial.println(WiFi.localIP()); 
        Serial.println("==================================");
    } else {
        Serial.println("\n[!] WiFi Connection Failed! Check Router.");
    }

    // تشغيل البلوتوث بعد ضمان استقرار الواي فاي تماماً لحل مشكلة تعليق الترددات
    delay(1000); 
    SerialBT.begin("ESP32_SmartFarm_BT");
    Serial.println("Bluetooth Initialized.");

    // تقديم الصفحة الرئيسية
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(wifiEnabled) {
            request->send_P(200, "text/html; charset=utf-8", index_html);
        } else {
            request->send(403, "text/plain", "Wi-Fi Access Blocked.");
        }
    });

    // تبادل البيانات الحية بصيغة JSON آمنة ومحمية
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"temp\":" + String(tempValue) + ",";
        json += "\"humid\":" + String(humidityValue) + ",";
        json += "\"soil\":" + String(soilMoisture) + ",";
        json += "\"rain\":" + String(rainValue) + ",";
        json += "\"pump\":" + String(pumpState ? "1" : "0") + ",";
        json += "\"heater\":" + String(heaterState ? "1" : "0") + ",";
        json += "\"alarm\":" + String(alarmState ? "1" : "0") + ",";
        json += "\"servo\":" + String(roofAngle) + ",";
        json += "\"wifi\":" + String(wifiEnabled ? "1" : "0") + ",";
        json += "\"bt\":" + String(bluetoothEnabled ? "1" : "0") + ",";
        json += "\"mqtt\":" + String(mqttEnabled ? "1" : "0") + ",";
        json += "\"mode\":" + String(isManualMode ? "1" : "0");
        json += "}";
        request->send(200, "application/json; charset=utf-8", json);
    });

    // استقبال تحديثات السلايدر مع حماية من القيم المفقودة (Anti-Crash Guard)
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
        if(wifiEnabled && isManualMode) {
            if(request->hasParam("type") && request->hasParam("value")) {
                String type = request->getParam("type")->value();
                float val = request->getParam("value")->value().toFloat();
                if(type == "temp") tempValue = val;
                else if(type == "humid") humidityValue = val;
                else if(type == "soil") soilMoisture = val;
                else if(type == "rain") rainValue = val;
                request->send(200, "text/plain", "OK");
                return;
            }
        }
        request->send(400, "text/plain", "Bad Request");
    });

    server.on("/toggleMode", HTTP_GET, [](AsyncWebServerRequest *request){
        isManualMode = !isManualMode;
        request->send(200, "text/plain", "OK");
    });

    server.on("/togglePerm", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("proto")) {
            String proto = request->getParam("proto")->value();
            if(proto == "wifi") wifiEnabled = !wifiEnabled;
            else if(proto == "bt") bluetoothEnabled = !bluetoothEnabled;
            else if(proto == "mqtt") mqttEnabled = !mqttEnabled;
            request->send(200, "text/plain", "OK");
            return;
        }
        request->send(400, "text/plain", "Missing Param");
    });

    server.begin();
    mqttClient.setServer(mqtt_server, 1883);
}

// --- الحلقة البرمجية المتكررة ---
void loop() {
    executeAutomationLogic();

    // تشغيل الـ MQTT بشكل ذكي لا يؤثر على خادم الويب المحلي
    if (mqttEnabled && WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            if (millis() - lastMqttReconnect > 8000) { // زيادة مؤقت إعادة الاتصال منعا للضغط
                lastMqttReconnect = millis();
                String clientId = "SmartFarm-" + String(random(0xffff), HEX);
                if (mqttClient.connect(clientId.c_str())) {
                    mqttClient.publish("farm/telemetry/status", "Online");
                }
            }
        } else {
            mqttClient.loop();
            if(millis() - lastMqttPub > 6000) {
                mqttClient.publish("farm/telemetry/temp", String(tempValue).c_str());
                mqttClient.publish("farm/telemetry/soil", String(soilMoisture).c_str());
                lastMqttPub = millis();
            }
        }
    }

    // استقبال بيانات البلوتوث
//    if (bluetoothEnabled && SerialBT.available()) {
//        char incomingChar = SerialBT.read();
//        if(incomingChar == 'P') {
//            pumpState = !pumpState;
//            digitalWrite(PUMP_PIN, pumpState);
//        }
//    }
      if (bluetoothEnabled) {
        
        // 1. استقبال وقراءة البيانات القادمة من التطبيق (أزرار وسلايدرات) دون حجز المعالج
        while (SerialBT.available()) {
            char incomingChar = SerialBT.read();
            
            if (incomingChar == '\n') { // عند استقبال نهاية السطر، نقوم بتحليل الأمر
                btBuffer.trim();
                if (btBuffer.length() > 0) {
                    
                    // أ) معالجة أوامر الأزرار المباشرة
                    if (btBuffer == "P") {
                        if (isManualMode) pumpState = !pumpState;
                    } 
                    else if (btBuffer == "H") {
                        if (isManualMode) heaterState = !heaterState;
                    } 
                    else if (btBuffer == "M") {
                        isManualMode = !isManualMode;
                    } 
                    // ب) معالجة قيم السلايدرات القادمة بصيغة (الحرف : القيمة)
                    else if (btBuffer.indexOf(':') != -1) {
                        int colonIdx = btBuffer.indexOf(':');
                        String type = btBuffer.substring(0, colonIdx);
                        float value = btBuffer.substring(colonIdx + 1).toFloat();
                        
                        if (isManualMode) { // لا نسمح بتغيير القراءات يدوياً إلا إذا كان الوضع يدوي
                            if (type == "T") tempValue = value;
                            else if (type == "U") humidityValue = value;
                            else if (type == "S") soilMoisture = value;
                            else if (type == "R") rainValue = value;
                        }
                    }
                }
                btBuffer = ""; // تصفير البافر لاستقبال الأمر القادم
            } else {
                btBuffer += incomingChar; // تجميع الحروف حاملاً السلسلة النصية
            }
        }

        // 2. بث القراءات الأربعة حية كل ثانية بصيغة CSV ليقرأها التطبيق ويقسمها بالفواصل
        if (SerialBT.hasClient() && (millis() - lastBtSend > 1000)) {
            lastBtSend = millis();
            
            // الترتيب الصارم: حرارة , رطوبة , تربة , مطر (مطابق للـ Indexes 1,2,3,4 في بلكاتك)
            String outData = String(tempValue, 1) + "," + 
                             String(humidityValue, 1) + "," + 
                             String(soilMoisture, 1) + "," + 
                             String(rainValue, 1);
            
            SerialBT.println(outData); // إرسال النص متبوعاً بـ \n لينتهي السطر في التطبيق
        }
    }
    delay(1); // إعطاء النظام فرصة تامة لتمرير حزم الـ WiFi بدون أدنى تأخير
}
