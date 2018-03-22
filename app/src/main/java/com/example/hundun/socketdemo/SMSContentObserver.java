package com.example.hundun.socketdemo;

import android.app.Activity;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by hundun on 2018/3/22.
 */

public class SMSContentObserver extends ContentObserver {
    //所有短信
    public static final String SMS_URI_ALL = "content://sms/";
    //收件箱短信
    public static final String SMS_URI_INBOX = "content://sms/inbox";
    //发件箱短信
    public static final String SMS_URI_SEND = "content://sms/sent";
    //草稿箱短信
    public static final String SMS_URI_DRAFT = "content://sms/draft";
    private Activity mActivity;
    private List<SmsInfo> mSmsInfoList;
    private MessageListener mMessageListener;

    public SMSContentObserver(Handler handler, Activity activity) {
        super(handler);
        this.mActivity = activity;
    }

    @Override
    public void onChange(boolean selfChange) {
        super.onChange(selfChange);
        Uri uri = Uri.parse(SMS_URI_INBOX);
        mSmsInfoList = this.getSmsInfo(uri,mActivity);
        mMessageListener.OnReceived(mSmsInfoList.get(0).getSmsbody());
        System.out.println("Message content is:"+mSmsInfoList.get(0).getSmsbody());
        System.out.println("Message info is:"+mSmsInfoList.get(0));
    }

    /**
     * 注意:
     * 该处只用按照时间降序取出第一条即可
     * 这条当然是最新收到的消息
     */
    private List<SmsInfo> getSmsInfo(Uri uri, Activity activity){
        List<SmsInfo> smsInfoList=new ArrayList<SmsInfo>();
        String[] projection = new String[] { "_id", "address", "person","body", "date", "type" };
        Cursor cusor = activity.managedQuery(uri, projection, null, null,"date desc limit 1");
        int nameColumn = cusor.getColumnIndex("person");
        int phoneNumberColumn = cusor.getColumnIndex("address");
        int smsbodyColumn = cusor.getColumnIndex("body");
        int dateColumn = cusor.getColumnIndex("date");
        int typeColumn = cusor.getColumnIndex("type");
        if (cusor != null) {
            while (cusor.moveToNext()) {
                SmsInfo smsinfo = new SmsInfo();
                smsinfo.setName(cusor.getString(nameColumn));
                smsinfo.setDate(cusor.getString(dateColumn));
                smsinfo.setPhoneNumber(cusor.getString(phoneNumberColumn));
                smsinfo.setSmsbody(cusor.getString(smsbodyColumn));
                smsinfo.setType(cusor.getString(typeColumn));
                smsInfoList.add(smsinfo);
            }
            cusor.close();
        }
        System.out.println("smsInfoList.size()="+smsInfoList.size());
        return smsInfoList;
    }



    // 回调接口
    public interface MessageListener {
        public void OnReceived(String message);
    }

    public void setOnReceivedMessageListener(
            MessageListener messageListener) {
        this.mMessageListener=messageListener;
    }
}
