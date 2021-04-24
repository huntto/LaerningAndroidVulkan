package com.ihuntto.tiny_engine.touch_pointer;

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

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private int mWidth;
    private int mHeight;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SurfaceView surfaceView = new SurfaceView(this);
        setContentView(surfaceView);

        setAssetManager(getAssets(), getDataDir().getAbsolutePath());
        surfaceView.getHolder().addCallback(mCallback);
        surfaceView.setOnTouchListener((view, motionEvent) -> {
            int action = motionEvent.getActionMasked();
            int actionIndex, id, pointerCount;
            float x, y, size;
            switch (action) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_POINTER_DOWN: {
                    actionIndex = motionEvent.getActionIndex();
                    id = motionEvent.getPointerId(actionIndex);

                    x = motionEvent.getX(actionIndex) / mWidth * 2 - 1;
                    y = -(motionEvent.getY(actionIndex) / mHeight * 2 - 1);
                    size = motionEvent.getTouchMajor(actionIndex);
                    updatePointer(id, x, y, size, 0);
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    pointerCount = motionEvent.getPointerCount();
                    for (int i = 0; i < pointerCount; i++) {
                        id = motionEvent.getPointerId(i);
                        x = motionEvent.getX(i) / mWidth * 2 - 1;
                        y = -(motionEvent.getY(i) / mHeight * 2 - 1);
                        size = motionEvent.getTouchMajor(i);
                        updatePointer(id, x, y, size, 1);
                    }
                    break;
                }
                case MotionEvent.ACTION_POINTER_UP:
                case MotionEvent.ACTION_UP: {
                    actionIndex = motionEvent.getActionIndex();
                    id = motionEvent.getPointerId(actionIndex);
                    x = motionEvent.getX(actionIndex) / mWidth * 2 - 1;
                    y = -(motionEvent.getY(actionIndex) / mHeight * 2 - 1);
                    size = motionEvent.getTouchMajor(actionIndex);
                    updatePointer(id, x, y, size, 2);
                    break;
                }
                default:
                    break;
            }
            draw();
            return true;
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
            mWidth = width;
            mHeight = height;
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

    private native void updatePointer(int index, float x, float y, float size, int action);
}