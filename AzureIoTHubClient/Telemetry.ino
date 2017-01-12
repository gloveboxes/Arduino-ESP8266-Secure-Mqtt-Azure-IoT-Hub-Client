//char buffer[256];
//
//char* serializeTelemetry(Sensor *telemetry) {
///*  https://github.com/bblanchon/ArduinoJson/wiki/Memory-model
//    Have allowed for a few extra json fields that actually being used at the moment
//*/
//  
//	StaticJsonBuffer<JSON_OBJECT_SIZE(16)> jsonBuffer;
//	JsonObject& root = jsonBuffer.createObject();
// 
//
//	root["Utc"] = getISODateTime();
//	root["Celsius"] = telemetry->temperature;
//	root["Humidity"] = telemetry->humidity;
//	root["hPa"] = telemetry->pressure;
//	root["Light"] = telemetry->light;
//	root["Geo"] = telemetry->geo;
//  root["Schema"] = 1;
//
//	//instrumentation
////	root["WiFi"] = telemetry->WiFiConnectAttempts;
//  root["Mem"] = ESP.getFreeHeap();
//	root["Id"] = ++telemetry->msgId;
//
//	root.printTo(buffer, sizeof(buffer));
//
//	return buffer;
//}
