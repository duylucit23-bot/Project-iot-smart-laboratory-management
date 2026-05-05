package com.labsecurity;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class Main {
    private static final String LOG_FILE = "log_phonglab.txt";
    private static MqttClient client; // Đưa client ra ngoài để các hàm khác có thể dùng

    public static void main(String[] args) {
        String broker = "tcp://broker.hivemq.com:1883"; 
        String clientId = "Java-Backend-Admin-" + System.currentTimeMillis(); 
        MemoryPersistence persistence = new MemoryPersistence();

        try {
            client = new MqttClient(broker, clientId, persistence);
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(true);

            System.out.println("Connecting to Broker...");
            client.connect(connOpts);
            System.out.println("OK: Ket noi MQTT thanh cong!");
            
            ghiLog("HE THONG", "Server Java khoi dong, ket noi MQTT thanh cong.");

            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    System.out.println("ERR: Mat ket noi MQTT!");
                    ghiLog("HE THONG", "Mat ket noi MQTT Broker!");
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    String payload = new String(message.getPayload());
                    System.out.println("MSG [" + topic + "]: " + payload);

                    try {
                        JsonObject jsonObject = JsonParser.parseString(payload).getAsJsonObject();

                        // --- XU LY NODE 2 (DHT11 & PIR) ---
                        if (topic.equals("lab/node2/status")) {
                            if (jsonObject.has("motion")) {
                                if (jsonObject.get("motion").getAsInt() == 1) {
                                    System.out.println(" -> CANH BAO: Co nguoi trong phong!");
                                    ghiLog("NODE 2", "Phat hien chuyen dong.");
                                }
                            }
                            if (jsonObject.has("temp")) {
                                float temp = jsonObject.get("temp").getAsFloat();
                                float hum = jsonObject.get("hum").getAsFloat();
                                System.out.println(" -> Nhiet do: " + temp + "C | Do am: " + hum + "%");
                                ghiLog("NODE 2", "Cap nhat -> T: " + temp + "C, H: " + hum + "%");

                                // --- LOGIC TU DONG DIEU KHIEN QUAT ---
                                if (temp >= 28.0) {
                                    sendControlCommand("FAN_ON");
                                    System.out.println(" -> [AUTO] Nhiet do cao (>=28C). Da gui lenh BAT QUAT.");
                                } else {
                                    sendControlCommand("FAN_OFF");
                                    System.out.println(" -> [AUTO] Nhiet do on dinh (<28C). Da gui lenh TAT QUAT.");
                                }
                            }
                        }

                        // --- XU LY NODE 1 (RFID & KEYPAD) ---
                        else if (topic.equals("lab/node1/auth")) {
                            if (jsonObject.has("rfid")) {
                                String rfid = jsonObject.get("rfid").getAsString();
                                System.out.println(" -> [RFID] Ma the: " + rfid);
                                if (rfid.equals("9E5D1305")) {
                                    System.out.println(" -> XAC NHAN: Chao Duy (Boss)!");
                                    ghiLog("NODE 1", "Mo cua bang the VIP: " + rfid);
                                } else {
                                    System.out.println(" -> TU CHOI: The la!");
                                    ghiLog("CANH BAO", "The la quet cua: " + rfid);
                                }
                            } 
                            else if (jsonObject.has("auth")) {
                                if (jsonObject.get("auth").getAsString().equals("password_success")) {
                                    System.out.println(" -> [KEYPAD] Mat khau dung. Cua mo!");
                                    ghiLog("NODE 1", "Mo cua bang Mat khau thanh cong.");
                                }
                            }
                        }

                    } catch (Exception e) {
                        System.out.println(" -> JSON Error: " + e.getMessage());
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {}
            });

            client.subscribe("lab/node2/status");
            client.subscribe("lab/node1/auth");
            System.out.println("System READY. Waiting for data...\n");

        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    // Hàm gửi lệnh điều khiển xuống Wemos
    private static void sendControlCommand(String command) {
        try {
            if (client != null && client.isConnected()) {
                MqttMessage message = new MqttMessage(command.getBytes());
                message.setQos(1);
                client.publish("lab/node2/control", message);
            }
        } catch (MqttException e) {
            System.out.println("Error publishing command: " + e.getMessage());
        }
    }

    private static void ghiLog(String nguon, String noiDung) {
        String timeStamp = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
        String dongLog = String.format("[%s] [%s] %s", timeStamp, nguon, noiDung);
        try (FileWriter fw = new FileWriter(LOG_FILE, true);
             BufferedWriter bw = new BufferedWriter(fw)) {
            bw.write(dongLog);
            bw.newLine();
        } catch (IOException e) {
            System.out.println("Error writing log: " + e.getMessage());
        }
    }
}