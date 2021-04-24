package com.ihuntto.android_vulkan.model;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SurfaceView surfaceView = new SurfaceView(this);
        setContentView(surfaceView);

        setAssetManager(getAssets(), getDataDir().getAbsolutePath());
        surfaceView.getHolder().addCallback(mCallback);
        surfaceView.setOnTouchListener(new View.OnTouchListener() {
            private float mPrevX;
            private float mPrevY;

            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "onTouch");
                int width = surfaceView.getWidth();
                int height = surfaceView.getHeight();
                switch (motionEvent.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                        mPrevX = motionEvent.getX() / width * 2 - 1;
                        mPrevY = -(motionEvent.getY() / height * 2 - 1);
                        break;
                    case MotionEvent.ACTION_MOVE:
                        float x = motionEvent.getX() / width * 2 - 1;
                        float y = -(motionEvent.getY() / height * 2 - 1);
                        float vx = x - mPrevX;
                        float vy = y - mPrevY;
                        mPrevX = x;
                        mPrevY = y;
                        if (Math.abs(vx) < 1e-6 && Math.abs(vy) < 1e-6) break;
                        float dist = (float) Math.sqrt(vx * vx + vy * vy);

                        rotate(dist, vy, vx, 0);
                        draw();
                        break;
                }
                return true;
            }
        });
    }

    private final SurfaceHolder.Callback mCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG, "surfaceCreated");
            init(holder.getSurface());
            draw();
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d(TAG, "surfaceChanged: width=" + width + " height=" + height);
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG, "surfaceDestroyed");
            cleanup();
        }
    };

    private native void init(@NonNull Surface surface);

    private native void cleanup();

    private native void setAssetManager(@NonNull AssetManager assetManager, String dataPath);

    private native void draw();

    private native void rotate(float radius, float x, float y, float z);
}