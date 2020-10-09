// Include required libraries
2. #include <MFRC522.h>
3. #include <LiquidCrystal_I2C.h>
4. #include <Keypad.h>
5. #include <SoftwareSerial.h>
6. #include <Servo.h>
7. #include <SPI.h>
8.
9. // Create instances
10. SoftwareSerial SIM900(3, 4); // SoftwareSerial SIM900(Rx, Tx)
11. MFRC522 mfrc522(10, 9); // MFRC522 mfrc522(SS_PIN, RST_PIN)
12. LiquidCrystal_I2C lcd(0x27, 16, 2);
13. Servo sg90;
14.
15. // Initialize Pins for led's, servo and buzzer
16. // Blue LED is connected to 5V
17. constexpr uint8_t greenLed = 7;
18. constexpr uint8_t redLed = 6;
19. constexpr uint8_t servoPin = 8;
20. constexpr uint8_t buzzerPin = 5;
21.
22. char initial_password[4] = {'1', '2', '3', '4'}; // Variable to store initial password
23. String tagUID = "29 B9 ED 23"; // String to store UID of tag. Change it with your tag's UID
24. char password[4]; // Variable to store users password
25. boolean RFIDMode = true; // boolean to change modes
26. boolean NormalMode = true; // boolean to change modes
27. char key_pressed = 0; // Variable to store incoming keys
28. uint8_t i = 0; // Variable used for counter
29.
30. // defining how many rows and columns our keypad have
31. const byte rows = 4;
32. const byte columns = 4;
33.
34. // Keypad pin map
35. char hexaKeys[rows][columns] = {
36. {'1', '2', '3', 'A'},
37. {'4', '5', '6', 'B'},
38. {'7', '8', '9', 'C'},
39. {'*', '0', '#', 'D'}
40. };
41.
42. // Initializing pins for keypad
43. byte row_pins[rows] = {A0, A1, A2, A3};
44. byte column_pins[columns] = {2, 1, 0};
45.
46. // Create instance for keypad
31
47. Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
48.
49. void setup() {
50. // Arduino Pin configuration
51. pinMode(buzzerPin, OUTPUT);
52. pinMode(redLed, OUTPUT);
53. pinMode(greenLed, OUTPUT);
54.
55. sg90.attach(servoPin); //Declare pin 8 for servo
56. sg90.write(0); // Set initial position at 0 degrees
57.
58. lcd.begin(); // LCD screen
59. lcd.backlight();
60. SPI.begin(); // Init SPI bus
61. mfrc522.PCD_Init(); // Init MFRC522
62.
63. // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
64. // Make sure that corresponds to the baud rate of your module
65. SIM900.begin(19200);
66.
67. // AT command to set SIM900 to SMS mode
68. SIM900.print("AT+CMGF=1\r");
69. delay(100);
70. // Set module to send SMS data to serial out upon receipt
71. SIM900.print("AT+CNMI=2,2,0,0,0\r");
72. delay(100);
73.
74. lcd.clear(); // Clear LCD screen
75. }
76.
77. void loop() {
78. if (NormalMode == false) {
79. // Function to receive message
80. receive_message();
81. }
82.
83. else if (NormalMode == true) {
84. // System will first look for mode
85. if (RFIDMode == true) {
86. // Function to receive message
87. receive_message();
88.
89. lcd.setCursor(0, 0);
90. lcd.print(" Door Lock");
91. lcd.setCursor(0, 1);
92. lcd.print(" Scan Your Tag ");
93.
94. // Look for new cards
95. if ( ! mfrc522.PICC_IsNewCardPresent()) {
96. return;
97. }
98.
99. // Select one of the cards
100. if ( ! mfrc522.PICC_ReadCardSerial()) {
32
101. return;
102. }
103.
104. //Reading from the card
105. String tag = "";
106. for (byte j = 0; j < mfrc522.uid.size; j++)
107. {
108. tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
109. tag.concat(String(mfrc522.uid.uidByte[j], HEX));
110. }
111. tag.toUpperCase();
112.
113. //Checking the card
114. if (tag.substring(1) == tagUID)
115. {
116. // If UID of tag is matched.
117. lcd.clear();
118. lcd.print("Tag Matched");
119. digitalWrite(greenLed, HIGH);
120. delay(3000);
121. digitalWrite(greenLed, LOW);
122.
123. lcd.clear();
124. lcd.print("Enter Password:");
125. lcd.setCursor(0, 1);
126. RFIDMode = false; // Make RFID mode false
127. }
128.
129. else
130. {
131. // If UID of tag is not matched.
132. lcd.clear();
133. lcd.setCursor(0, 0);
134. lcd.print("Wrong Tag Shown");
135. lcd.setCursor(0, 1);
136. lcd.print("Access Denied");
137. digitalWrite(buzzerPin, HIGH);
138. digitalWrite(redLed, HIGH);
139. send_message("Someone Tried with the wrong tag \nType 'close' to halt the system.");
140. delay(3000);
141. digitalWrite(buzzerPin, LOW);
142. digitalWrite(redLed, LOW);
143. lcd.clear();
144. }
145. }
146.
147. // If RFID mode is false, it will look for keys from keypad
148. if (RFIDMode == false) {
149. key_pressed = keypad_key.getKey(); // Storing keys
150. if (key_pressed)
151. {
152. password[i++] = key_pressed; // Storing in password variable
153. lcd.print("*");
154. }
33
155. if (i == 4) // If 4 keys are completed
156. {
157. delay(200);
158. if (!(strncmp(password, initial_password, 4))) // If password is matched
159. {
160. lcd.clear();
161. lcd.print("Pass Accepted");
162. sg90.write(90); // Door Opened
163. digitalWrite(greenLed, HIGH);
164. send_message("Door Opened \nIf it was't you, type 'close' to halt the system.");
165. delay(3000);
166. digitalWrite(greenLed, LOW);
167. sg90.write(0); // Door Closed
168. lcd.clear();
169. i = 0;
170. RFIDMode = true; // Make RFID mode true
171. }
172. else // If password is not matched
173. {
174. lcd.clear();
175. lcd.print("Wrong Password");
176. digitalWrite(buzzerPin, HIGH);
177. digitalWrite(redLed, HIGH);
178. send_message("Someone Tried with the wrong Password \nType 'close' to halt the system.");
179. delay(3000);
180. digitalWrite(buzzerPin, LOW);
181. digitalWrite(redLed, LOW);
182. lcd.clear();
183. i = 0;
184. RFIDMode = true; // Make RFID mode true
185. }
186. }
187. }
188. }
189. }
190.
191. // Receiving the message
192. void receive_message()
193. {
194. char incoming_char = 0; //Variable to save incoming SMS characters
195. String incomingData; // for storing incoming serial data
196.
197. if (SIM900.available() > 0)
198. {
199. incomingData = SIM900.readString(); // Get the incoming data.
200. delay(10);
201. }
202.
203. // if received command is to open the door
204. if (incomingData.indexOf("open") >= 0)
205. {
206. sg90.write(90);
207. NormalMode = true;
208. send_message("Opened");
34
209. delay(10000);
210. sg90.write(0);
211. }
212.
213. // if received command is to halt the system
214. if (incomingData.indexOf("close") >= 0)
215. {
216. NormalMode = false;
217. send_message("Closed");
218. }
219. incomingData = "";
220. }
221.
222. // Function to send the message
223. void send_message(String message)
224. {
225. SIM900.println("AT+CMGF=1"); //Set the GSM Module in Text Mode
226. delay(100);
227. SIM900.println("AT+CMGS=\"+919564151906\""); //
228. delay(100);
229. SIM900.println(message); // The SMS text you want to send
230. delay(100);
231. SIM900.println((char)26); // ASCII code of CTRL+Z
232. delay(100);
233. SIM900.println();
234. delay(1000);
235. }