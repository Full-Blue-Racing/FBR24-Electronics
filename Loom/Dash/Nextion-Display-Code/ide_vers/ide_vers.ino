#include <EasyNextionLibrary.h>
#include <SPI.h>
#include <mcp_can.h>


EasyNex myNex(Serial);
int data_buffer[48] = { 0xFFFFFFFFFFFF };
MCP_CAN CAN0(10);




void sendChartData(const char * id, const char * units,int num_bytes,int location_in_buffer,char data_string_buffer[20],char units_buffer[18]){
  String id_as_string = id;
  int data_from_buffer = getFromBuffer(location_in_buffer,num_bytes,data_buffer);
  itoa(data_from_buffer, data_string_buffer, 10);
  units_buffer[0] = *units;
  data_string_buffer[2] = *units_buffer;
  String data_string = data_string_buffer;
  myNex.writeStr(id_as_string,data_string);
  Serial.println("\n");

}
void updateTachometer(int data_buffer[48],EasyNex myNex){
    
    uint16_t value = getFromBuffer(2,2,data_buffer);

    uint32_t number_of_images = 208;     //There are actually 209 but the 0th image doesnt count

    uint32_t offset = 2;
    uint32_t image_number;
    if (value>16){
        image_number = number_of_images;
    }else{
        image_number = value * (16/number_of_images);
    }
    image_number += offset;

    myNex.writeNum("tachometer.pic",image_number);
}


void updateSpeedometer(int data_buffer[48],EasyNex myNex){

    uint16_t value = 0;


    uint32_t number_of_images = 270;     //There are actually 271 but the 0th image doesnt count
    uint32_t offset = 211;
    uint32_t image_number;
     
    if (value>120){
        image_number = number_of_images;
    }else{
        image_number = value * (120/number_of_images);
    }
    image_number += offset;

    myNex.writeNum("speedometer.pic",image_number);
}

void updateStickPosition(int data_buffer[48], EasyNex myNex){

    uint16_t position = getFromBuffer(6,2,data_buffer);

    uint8_t in_red_bound = 90; //change this is not temp but value on progress bar
    uint8_t in_orange_band = 80; //"" ""

    
    myNex.writeNum("temp_guage.val",position);
    
    int colour = 1024;  //Colour value for a visual warning on the guage -inital = green
    if (position > in_red_bound){
        colour = 40960; //red
    }else if (position > in_orange_band){
        colour = 45504; //orange
    };
    myNex.writeNum("temp_guage.pco",colour);
}


void updateTemp(int data_buffer[48], EasyNex myNex){

    uint16_t value = getFromBuffer(4,2,data_buffer);


    int in_red_bound = 90; //change this is not temp but value on progress bar
    int in_orange_band = 80; //"" ""
    int max_val = 200; // The maximum value of temperature to be 100 on the progress bar

    int scaled_for_bar_value = value * (100/max_val);
    if (scaled_for_bar_value > 100){
        scaled_for_bar_value = 100;
    };
    myNex.writeNum("temp_guage.val",scaled_for_bar_value);
    
    int colour = 1024;  //Colour value for a visual warning on the guage -inital = green
    if (scaled_for_bar_value > in_red_bound){
        colour = 40960; //red
    }else if (scaled_for_bar_value > in_orange_band){
        colour = 45504; //orange
    };
    myNex.writeNum("temp_guage.pco",colour);
}
void updateManifoldAirTemperature(int data_buffer[48],EasyNex myNex){ //Turns warning light on/off

    int temp = getFromBuffer(12,2,data_buffer);

    int max_safe_val = 100;
    int min_safe_val = 0;
    int colour = 1024; //green
    if ((temp> max_safe_val)||(temp < min_safe_val)){
        colour = 40960; //red
    }
    myNex.writeNum("coolant_temp.pco",colour);

}

void updateAirPressure(int data_buffer[48], EasyNex myNex){ //Turns warning light on/of 

    uint16_t pressure = getFromBuffer(0,2,data_buffer);

    uint16_t max_safe_val = 120;
    uint16_t min_safe_val = 50;
    int colour = 1024; //green
    if ((pressure> max_safe_val)||(pressure < min_safe_val)){
        colour = 40960; //red
    }
    myNex.writeNum("air_pressure.pco",colour);

}

void updateBatteryVoltage(int data_buffer[48], EasyNex myNex){ //Turns warning light on/off

    uint16_t voltage = getFromBuffer(24,2,data_buffer);

    uint16_t max_safe_val = 12.5;
    uint16_t min_safe_val = 11.5;
    int colour = 1024; //green
    if ((voltage> max_safe_val)||(voltage < min_safe_val)){
        colour = 40960; //red
    }

    myNex.writeNum("battery_volts.pco",colour);
}
void setWarningLights(int data_buffer[48],EasyNex myNex){ //Just runs the updates only for simplification
    updateAirPressure(data_buffer, myNex);
    updateManifoldAirTemperature(data_buffer, myNex);
    updateBatteryVoltage(data_buffer,myNex);
}



void updateHomePage(EasyNex myNex,int data_buffer[48]){



        setWarningLights(data_buffer, myNex);
        updateTachometer(data_buffer, myNex);
        updateSpeedometer(data_buffer, myNex);
        updateStickPosition(data_buffer,myNex);
        updateTemp(data_buffer,myNex);



}

int convertToGraphValue(int min, int max, int value){
    int max_return_value = 5*256/6;

    max -= min;



    value = value * (max_return_value/max);
    value += min;
    return value;
}
void sendValueToGraph(String variable_name, int value, EasyNex myNex){
    myNex.writeNum(variable_name,value);
}
void updateBatteryGraph(int data_buffer[48],EasyNex myNex){
    int batt_volts = getFromBuffer(24,2,data_buffer);
    int max = 13;
    int min = 11;
    int graph_value = convertToGraphValue(min,max,batt_volts);
    sendValueToGraph("bat_volts.val",graph_value,myNex);
}
void updateRevsGraph(int data_buffer[48],EasyNex myNex){
    int revs = getFromBuffer(2,2,data_buffer);
    int max = 16000;
    int min = 0;
    int graph_value = convertToGraphValue(min,max,revs);
    sendValueToGraph("revs_var.val",graph_value,myNex);
}
void updateEngineTempGraph(int data_buffer[48],EasyNex myNex){
    int temp = 100;//getFromBuffer(4,2,data_buffer);
    int max = 200;
    int min = 0;
    int graph_value = convertToGraphValue(min,max,temp);
    sendValueToGraph("temp_var.val",graph_value,myNex);
}
void updateVelocityGraph(int data_buffer[48],EasyNex myNex){
    int revs = getFromBuffer(0,2,data_buffer);
    int max = 255;
    int min = 0;
    int graph_value = convertToGraphValue(min,max,revs);
    sendValueToGraph("vel_var.val",graph_value, myNex);
}
void updateGraphs(int data_buffer[48],EasyNex myNex){
    updateBatteryGraph(data_buffer,myNex);
    updateRevsGraph(data_buffer,myNex);
    updateEngineTempGraph(data_buffer,myNex);
    updateVelocityGraph(data_buffer,myNex);
}





bool startCan(MCP_CAN CAN0){
    #define CAN0_INT 2

    if(!(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ)==CAN_OK)){
        return 0;
    }
    else{
        CAN0.setMode(MCP_NORMAL);
        pinMode(CAN0_INT, INPUT);
        return 1;
    };
}



void set_data(int data_buffer[48],unsigned char rxBuf[8], long unsigned int rxID){
    
    int pointer_offset = (rxID - 1512)*8;
    int * pointer_start = data_buffer+pointer_offset;
    
    int byte_in_buf = 0;
     
    while (byte_in_buf<8){
        *(pointer_start+byte_in_buf) = *(rxBuf+byte_in_buf);
        
        byte_in_buf++;
    }
}


void readCanDataInBuffer(int data_buffer[48],MCP_CAN CAN0){
    long unsigned int rxID;
    unsigned char len=0;
    unsigned char rxBuf[8];

    #define CAN0_INT 2

    if (!digitalRead(CAN0_INT)){
        CAN0.readMsgBuf(&rxID, &len, rxBuf);
        set_data(data_buffer,rxBuf, rxID); 
    }
    
    
}
int getFromBuffer(int pointer_offset,int num_bytes,int data_buffer[48]){
    int data_from_buffer = 0;
    double scale_factor_list[48] = {0.1,0,1,0,0.1,0,0.1,0,0.001,0,0.001,0,10,0,10,0,10,10,10,0,10,0,0.001,0,10,0,10,0,10,0,10,10,0,10,0,10,0,1,1,1,1};

    for (int byte=0;byte==num_bytes; byte ++){
    data_from_buffer += data_buffer[pointer_offset] << 8*byte;
    }

    data_from_buffer = data_from_buffer * scale_factor_list[pointer_offset];
    return data_from_buffer;
}

void updateDataChart(){
  char data_string_buffer[20];
  char units[18];

  Serial.println("yoooo");
  sendChartData("map.val"," kPa",2, 0, data_string_buffer, units);
  sendChartData("rpm.val"," RPM", 2,2, data_string_buffer, units);
  sendChartData("clt.val"," deg F",2, 4, data_string_buffer, units);
  sendChartData("tps.val"," deg BTDC",2, 6, data_string_buffer, units);
  sendChartData("pw1.val"," AFR",2, 8, data_string_buffer, units);
  sendChartData("pw2.val"," AFR",2, 10, data_string_buffer, units);
  sendChartData("mat.val"," %",2, 12, data_string_buffer, units);
  sendChartData("adv_deg.val"," deg F",2, 14, data_string_buffer, units);
  sendChartData("afrtg1.val"," ms",1, 16, data_string_buffer, units);
  sendChartData("afr1.val"," V",1, 17, data_string_buffer, units);
  sendChartData("egocor1.val"," deg",2, 18, data_string_buffer, units);
  sendChartData("egt1.val"," m/s",2, 20, data_string_buffer, units);
  sendChartData("pwseq1.val"," deg",2, 22, data_string_buffer, units);
  sendChartData("batt.val"," deg",2, 24, data_string_buffer, units);
  sendChartData("knk_rtd.val"," ",1, 30, data_string_buffer, units);
  sendChartData("vss1.val"," ",2, 32, data_string_buffer, units);
  sendChartData("tc_retard.val"," ",2, 34, data_string_buffer, units);
  sendChartData("launch_timing.val"," ",2, 36, data_string_buffer, units);
  sendChartData("sw.val"," ",1, 40, data_string_buffer, units);
  sendChartData("tp.val"," ",2, 41, data_string_buffer, units);
  sendChartData("nsl1.val"," ",2, 43, data_string_buffer, units);
  sendChartData("gp.val"," ",2, 45, data_string_buffer, units);

}


void setup(){
  myNex.begin(9600);
  while (startCan(CAN0)){
    Serial.println("Waiting for CAN");
  };
  delay(500);

}

void loop(){
  myNex.NextionListen();
  int page = myNex.readNumber("dp");
  readCanDataInBuffer(data_buffer,CAN0);

  if (page == 1){
    updateHomePage(myNex,data_buffer);
  }
  else if (page == 7 || page == 8 || page == 9){
    updateDataChart();
  }
  else if (!(page == 0)) {
    updateGraphs(data_buffer,myNex);
  }

}


