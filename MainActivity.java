package com.example.nichelle.petometer;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.UUID;

import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "BT Data Comm";
    TextView Accelerometer;
    TextView Temperature;

    Handler h;

    final int RECEIVE_MESSAGE = 1;		// Status  for Handler
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private OutputStream outStream = null;
    private StringBuilder sb = new StringBuilder();
    private ConnectedThread mConnectedThread;
    // SPP UUID service. DO NOT CHANGE!!! This is a Standard SerialPortService ID per
    // https://developer.android.com/reference/android/bluetooth/BluetoothDevice.html#createRfcommSocketToServiceRecord%28java.util.UUID%29
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // MAC-address of HC-05
    private static String address = "98:D3:31:FC:0F:5F";
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Accelerometer = (TextView) findViewById(R.id.Accelerometer);
        Temperature = (TextView) findViewById(R.id.Temperature);
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        checkBTState();
            h = new Handler() {
                String bla = "";
                public void handleMessage(android.os.Message msg) {
                    switch (msg.what) {
                        case RECEIVE_MESSAGE:                                    // if receive message
                            byte[] readBuf = (byte[]) msg.obj;
//                            char[] sensor = new char[26];
//                            int index = 0;
                            //String readBuf = (String) msg.obj;
                            String strIncom = new String(readBuf, 0, msg.arg1);    // create string from bytes array
                            sb.append(strIncom);                                // append string
                            //int endOfLineIndex = sb.indexOf("@");                // determine the end-of-line
                            //if (endOfLineIndex > 0) {                            // if end-of-line,
                                //String sbprint1 = sb.substring(0, endOfLineIndex);
                                //Accelerometer.setText(sbprint1);
                               //String bla = "";
                                if(sb.charAt(0) == '0' ||sb.charAt(0) == '1'||sb.charAt(0) == '2'||sb.charAt(0) == '3'||
                                sb.charAt(0) == '4'||sb.charAt(0) == '5'||sb.charAt(0) == '6'||sb.charAt(0) == '7'||sb.charAt(0) == '8'||sb.charAt(0) == '9') {
                                    bla += sb.substring(0, 1);
                                }
                                else if(sb.charAt(0) == '$' && bla != ""){
                                    Accelerometer.setText(bla);
                                    bla = "";
                                }
                                else if(sb.charAt(0) == '*' && bla != ""){
                                        if(bla.length() == 2) {
                                            Temperature.setText(bla + "°F");
                                            bla = "";
                                        }
                                }
//                                    if(sb.charAt(0) == '$')
//                                    {
//                                        Accelerometer.setText(bla);
//                                        bla = "";
//                                    }
//                                    else {
//                                        bla += sb.substring(0, 1);
//                                        //Accelerometer.setText(bla);
//                                    }


                                sb.delete(0,sb.length());
                            //}
                            Log.d(TAG, "MsgString:" + sb.toString() + "Byte:" + msg.arg1 + "...");
                            break;
                    }
                }
            };
        btAdapter = BluetoothAdapter.getDefaultAdapter();		// get Bluetooth adapter
        checkBTState();
    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        if(Build.VERSION.SDK_INT >= 10){
            try {
                final Method m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord",
                        new Class[] { UUID.class });
                return (BluetoothSocket) m.invoke(device, MY_UUID);
            } catch (Exception e) {
                Log.e(TAG, "Could not create Insecure RFComm Connection",e);
            }
        }
        return  device.createRfcommSocketToServiceRecord(MY_UUID);
    }

    @Override
    public void onResume() {
        super.onResume();

        Log.d(TAG, "onResume(): Creating bluetooth socket ...");

        // Set up a pointer to the remote node using it's address.
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        // Two things are needed to make a connection:
        //   A MAC address, which we got above.
        //   A Service ID or UUID.  In this case we are using the
        //     UUID for SPP.

        try {
            btSocket = createBluetoothSocket(device);
            Log.d(TAG, "onResume(): Bluetooth socket created ...");
        } catch (IOException e) {
            errorExit("Fatal Error", "onResume(): Create bluetooth socket FAILED: " + e.getMessage() + ".");
        }

        // Discovery is resource intensive.  Make sure it isn't going on
        // when you attempt to connect and pass your message.
        btAdapter.cancelDiscovery();

        // Establish the connection.  This will block until it connects.
        Log.d(TAG, "Connecting to Bluetooth Device ...");
        try {
            btSocket.connect();
            Log.d(TAG, "Bluetooth Device Connected ...");
        } catch (IOException e) {
            try {
                btSocket.close();
            } catch (IOException e2) {
                errorExit("Fatal Error", "onResume(): Unable to close socket when closing connection: " + e2.getMessage() + ".");
            }
        }

//        // Create a data stream so we can talk to server.
        Log.d(TAG, "onResume(): Creating data output stream ...");
//        try {
//            outStream = btSocket.getOutputStream();
//        } catch (IOException e) {
//            errorExit("Fatal Error", "onResume(): Creating data output stream FAILED: " + e.getMessage() + ".");
//        }
        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();
    }


    @Override
    public void onPause() {
        super.onPause();

        Log.d(TAG, "Inside onPause()...");

        try     {
            btSocket.close();
        } catch (IOException e2) {
            errorExit("Fatal Error", "onPause(): FAILED to close socket." + e2.getMessage() + ".");
        }
    }

    private void checkBTState() {
        // Check for Bluetooth support and then check to make sure it is turned on
        // Emulator doesn't support Bluetooth and will return null
        if(btAdapter==null) {
            errorExit("Fatal Error", "Bluetooth not supported");
        } else {
            if (btAdapter.isEnabled()) {
                Log.d(TAG, "Bluetooth is ON...");
            } else {
                //Prompt user to turn on Bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }

    private void errorExit(String title, String message){
        Toast.makeText(getBaseContext(), title + " - " + message, Toast.LENGTH_LONG).show();
        finish();
    }

    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[256];  // buffer store for the stream
            int bytes; // bytes returned from read()

            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);		// Get number of bytes and message in "buffer"
                    h.obtainMessage(RECEIVE_MESSAGE, bytes, -1, buffer).sendToTarget();		// Send to message queue Handler
                } catch (IOException e) {
                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(String message) {
            Log.d(TAG, "Info: ConnectedThread: data to send: " + message + "...");
            byte[] msgBuffer = message.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                Log.d(TAG, "Error: ConnectedThread: data to send: " + e.getMessage() + "...");
            }
        }
    }
}
 