package com.example.hundun.socketdemo;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class MainActivity extends Activity{

    private final String HOST_IP="192.168.4.1";
    private final int HOST_PORT=2121;

    Button btn_accept;
    EditText txtCommand;
    EditText txtEcho;

    public class MyThread extends Thread
    {
        private String command;
        EditText txtEcho;

        public MyThread(String command,EditText txtEcho)
        {
            this.command=command;
            this.txtEcho=txtEcho;
            //Thread thread =this;
            //thread.run();
        }

        @Override
        public void run() {
            try {
                String echo=acceptServer(command);
                txtEcho.append("\n"+echo);
            } catch (Exception e) {
                e.printStackTrace();
                //Toast.makeText(getApplicationContext(), e.getMessage(),Toast.LENGTH_SHORT).show();
            }
        }

    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btn_accept = (Button) findViewById(R.id.btn_accept);
        txtCommand=(EditText) findViewById(R.id.txtCommand);
        txtEcho=(EditText) findViewById(R.id.txtEcho);

        btn_accept.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String command=txtCommand.getText().toString();
                txtCommand.setText("");
                Thread thread=new MyThread(command,txtEcho);
                thread.start();
            }});

        Toast.makeText(getApplicationContext(), "init",Toast.LENGTH_SHORT).show();
    }


    private String acceptServer(String command) throws IOException {
        String echo;
        //1.创建客户端Socket，指定服务器地址和端口
        Socket socket = new Socket(HOST_IP, HOST_PORT);
        //2.获取输出流，向服务器端发送信息
        OutputStream os = socket.getOutputStream();//字节输出流
        PrintWriter pw = new PrintWriter(os);//将输出流包装为打印流

        BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        //获取客户端的IP地址
        InetAddress address = InetAddress.getLocalHost();
        String ip = address.getHostAddress();
        pw.write(command);
        pw.flush();



        try{
            //从服务器端接收数据有个时间限制（系统自设，也可以自己设置），超过了这个时间，便会抛出该异常
            echo= br.readLine();
            //txtCommand.append("\n"+echo);
            //Toast.makeText(MainActivity.this, echo,Toast.LENGTH_SHORT).show();
        }catch(SocketTimeoutException e){
            echo="Time out, No response";
            //txtCommand.append("\n"+"Time out, No response");
            //Toast.makeText(MainActivity.this, "Time out, No response",Toast.LENGTH_SHORT).show();
        }

        if(socket != null){
            //如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            socket.shutdownOutput();//关闭输出流
            socket.close(); //只关闭socket，其关联的输入输出流也会被关闭
        }

        return echo;

    }
}
