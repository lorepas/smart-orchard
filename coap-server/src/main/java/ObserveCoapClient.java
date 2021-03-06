import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.TimeUnit;
import java.sql.Timestamp;

import org.eclipse.californium.core.*;
import org.json.simple.*;

public class ObserveCoapClient extends CoapClient {
	private Sprinkler spr;
	private HumiditySensor hum_sen;
	private TemperatureSensor tem_sen;
	private String key_hum;
	private String key_tem;
	private String key_spr;
	CoapObserveRelation cor;
	

	public ObserveCoapClient(Sprinkler s) {
		super(s.getResURI());
		this.spr = new Sprinkler(s.getPath(), s.getAdd(), s.getOrchard());
		this.key_spr = "sprinkler_"+s.getAdd();
	}
	
	public ObserveCoapClient(HumiditySensor hs) {
		super(hs.getResURI());
		this.hum_sen = new HumiditySensor(hs.getPath(), hs.getAdd(), hs.getOrchard());
		this.key_hum = "hum_"+hs.getAdd();
	}
	
	public ObserveCoapClient(TemperatureSensor ts) {
		super(ts.getResURI());
		this.tem_sen = new TemperatureSensor(ts.getPath(), ts.getAdd(), ts.getOrchard());
		this.key_tem = "temp_"+ts.getAdd();
	}
		
	public void startCoapObserve() {
		cor = this.observe(new CoapHandler() {
			public void onLoad(CoapResponse response) {
				try {
					JSONObject jsonOb = (JSONObject) JSONValue.parseWithException(response.getResponseText());
					if(jsonOb.containsKey("hum_value")) {
						int hum_value = Integer.parseInt(jsonOb.get("hum_value").toString());
						int hum_thr = Integer.parseInt(jsonOb.get("thr_hum").toString());
						App.hum_sensor.get(key_hum).setHumidity_threshold(hum_thr);
						App.hum_sensor.get(key_hum).setValue(hum_value);
						hum_sen = App.hum_sensor.get(key_hum);
					} else if(jsonOb.containsKey("temp_value")) {
						int temp_value = Integer.parseInt(jsonOb.get("temp_value").toString());
						int temp_thr = Integer.parseInt(jsonOb.get("thr_tmp").toString());
						App.temp_sensor.get(key_tem).setTemperature_threshold(temp_thr);
						App.temp_sensor.get(key_tem).setValue(temp_value);
						tem_sen = App.temp_sensor.get(key_tem);
					} else if(jsonOb.containsKey("active")) {
						String sprinkling = jsonOb.get("sprinkling").toString();
						String active = jsonOb.get("active").toString();
						if(active.contains("ON"))
							App.sprinkler.get(key_spr).setActive(true);
						else
							App.sprinkler.get(key_spr).setActive(false);
						
						if(sprinkling.contains("YES"))
							App.sprinkler.get(key_spr).setSprinkling(true);
						else
							App.sprinkler.get(key_spr).setSprinkling(false);
						spr = App.sprinkler.get(key_spr);
					}

					if(App.obs==true && hum_sen != null) {
						Date date = new Date();
						long time = date.getTime();
						Timestamp t = new Timestamp(time);
						String[] number_node = hum_sen.getAdd().split(":");
						System.out.println("\n-----------------------------");
						System.out.println("--------- TIMESTAMP NODE ("+number_node[number_node.length-1]+")|| "+t);
						System.out.println("-----------------------------\n");
						System.out.println("The orchard is:\t"+hum_sen.getOrchard().toUpperCase()+"\n");
						System.out.println(hum_sen);
					}

				}catch(Exception e){
					e.printStackTrace();
				}
			}
			
			public void onError() {
				System.out.println("-------- REQ TIMEOUT OR REJECT OBSERVING --------\n");
			}
		});
	}
}
