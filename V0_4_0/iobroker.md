// ioBroker JavaScript für Lichtsteuerung
// Wird ausgelöst, wenn ein Befehl auf esp32solar/light/1/command eintrifft
on({id: 'mqtt.0.esp32solar.light.1.command', change: 'any'}, function(obj) {
    var value = obj.state.val;
    
    // Befehl ausführen (z.B. über Homematic, Zigbee oder andere Adapter)
    // Beispiel: setState('hm-rpc.0.KEQ0123456.1.STATE', value === 'ON');
    
    // Bestätigung zurücksenden
    setState('mqtt.0.esp32solar.light.1.status', value);
    
    log('Licht wurde geschaltet: ' + value);
});



7. Struktur im ioBroker
In ioBroker solltest du folgende Struktur anlegen:

Objekte unter mqtt.0:

esp32solar/light/1/command (für Befehle vom ESP32 an ioBroker)
esp32solar/light/1/status (für Statusrückmeldungen von ioBroker zum ESP32)
esp32solar/rolladen/1/command
esp32solar/rolladen/1/status
usw.