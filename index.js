const express = require('express');
const app = express();
const PORT = 3000;
const WebSocket = require('ws');

// تقديم الملفات الثابتة (CSS وغيرها)
app.use(express.static(__dirname + '/views/public'));


// تقديم الموقع
app.get('/', function(req, res) {
    res.sendFile(__dirname + "/views/index.html");
});

// تشغيل الخادم
const server = app.listen(PORT, function() {
    console.log("Server running on port " + PORT);
});

// إعداد WebSocket
const wss = new WebSocket.Server({ server });

let button1_status = "LED1off"; // الحالة الافتراضية للزر الأول
let button2_status = "LED2off"; // الحالة الافتراضية للزر الثاني

// التعامل مع الاتصالات
wss.on('connection', function(ws) {
    console.log('Client connected');

    // إرسال الحالة الحالية عند الاتصال
    ws.send(button1_status);
    ws.send(button2_status);

    ws.on('message', function(msg) {
        console.log("Received:", msg);

        // تحديث الحالة بناءً على الرسالة
        if (msg.startsWith("LED1")) {
            button1_status = msg;
        } else if (msg.startsWith("LED2")) {
            button2_status = msg;
        }

        // إذا كان هناك تسريب للغاز (مثال: رسالة "Gas leak detected!")
        if (msg === "fire detected!") {
            // إرسال إشعار إلى جميع العملاء
            wss.clients.forEach(function(client) {
                if (client.readyState === WebSocket.OPEN) {
                    client.send("fire detected!"); // إشعار التسريب
                }
            });
        }

        // إذا كانت الرسالة تحتوي على قيمة MQ2
        if (msg.startsWith("mq2value:")) {
            // إرسال قيمة MQ2 إلى جميع العملاء المتصلين
            wss.clients.forEach(function(client) {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(msg); // إرسال رسالة MQ2
                }
            });
        }

        // إرسال الرسالة لجميع العملاء المتصلين
        wss.clients.forEach(function(client) {
            if (client.readyState === WebSocket.OPEN) {
                client.send(msg);
            }
        });
    });

    ws.on('close', function() {
        console.log('Client disconnected');
    });
});
