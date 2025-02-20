#include <M5AtomS3.h>
#include <IRremote.h>
#include <MIDI.h>

// ONの場合C1(36),velocity100
// OFFの場合C2(48),velocity100

// IR送信ピンの定義
const int IR_SEND_PIN = 4;  // AtomS3のIRピン

// MIDIシリアルポートの設定
struct Serial2MIDISettings : public midi::DefaultSettings {
    static const long BaudRate = 31250;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI, Serial2MIDISettings);

// IR信号のデータ
uint16_t rawDataOn[19] = {2554, 2688, 842, 858, 840, 860, 838, 1910, 838, 1934, 812, 1936, 836, 1910, 836, 864, 836, 864, 812};
uint16_t rawDataOff[19] = {2554, 2686, 842, 858, 840, 1908, 840, 860, 838, 864, 836, 1934, 812, 886, 814, 1934, 814, 1934, 812};

bool lastButtonState = false;

void setup() {
    M5.begin();  // Serial, I2C, Display
    Serial.begin(115200);
    delay(50);  // 起動待ち

    // MIDI設定
    Serial2.begin(31250, SERIAL_8N1, 1, 2);   
    MIDI.begin(MIDI_CHANNEL_OMNI);  // すべてのチャンネルを受信
    
    // IR送信の初期化
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
    
    Serial.println("System initialized");
}

void loop() {
    M5.update();
    
    // ボタン制御
    bool currentButtonState = M5.BtnA.isPressed();
    if (currentButtonState != lastButtonState) {
        if (currentButtonState) {
            Serial.println("Button pressed - Sending IR ON");
            IrSender.sendRaw(rawDataOn, sizeof(rawDataOn) / sizeof(rawDataOn[0]), 38);
        } else {
            Serial.println("Button released - Sending IR OFF");
            IrSender.sendRaw(rawDataOff, sizeof(rawDataOff) / sizeof(rawDataOff[0]), 38);
        }
        lastButtonState = currentButtonState;
    }

    // MIDI受信処理
    if (MIDI.read()) {
        byte type = MIDI.getType();
        byte note = MIDI.getData1();
        byte velocity = MIDI.getData2();

        if (type == midi::NoteOn && velocity > 0) {
            Serial.printf("MIDI Note On: %d, Velocity: %d\n", note, velocity);
            
            if (note == 36) {
                Serial.println("Sending IR ON signal");
                IrSender.sendRaw(rawDataOn, sizeof(rawDataOn) / sizeof(rawDataOn[0]), 38);
            }
            else if (note == 48) {
                Serial.println("Sending IR OFF signal");
                IrSender.sendRaw(rawDataOff, sizeof(rawDataOff) / sizeof(rawDataOff[0]), 38);
            }
        }
    }
    
    delay(10);
}