package org.chromium.chrome.browser.kilo;

import android.app.AlertDialog;
import android.content.Context;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import org.chromium.base.Log;

import androidx.preference.Preference;

import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;

import org.chromium.chrome.R;

import org.chromium.chrome.browser.shields.FilterListServiceFactory;

public class KiloRPCEditPreference extends Preference {

    private static final String TAG = "KiloRPCEditPreference";

    public KiloRPCEditPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onClick() {
        super.onClick();
        showInputDialog();
    }

    private void showInputDialog() {
        Context context = getContext();

        FilterListServiceFactory factory = FilterListServiceFactory.getInstance();
        String currentGateway = factory.getRPCGateWay();
        if (currentGateway == null) currentGateway = "now the gateway is null";

        Log.d(TAG, "get rpc gateway: " + currentGateway);

        TextInputLayout inputLayout = new TextInputLayout(context);
        inputLayout.setHint("Set rpc gateway");

        TextInputEditText editText = new TextInputEditText(context);
        editText.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
        editText.setText(currentGateway);
        editText.setSelection(currentGateway.length()); 

        inputLayout.addView(editText);

        new AlertDialog.Builder(context)
                .setTitle(getTitle())
                .setView(inputLayout)
                .setPositiveButton("OK", (dialog, which) -> {
                    String value = editText.getText().toString().trim();
                    if (!value.isEmpty()) {
                        onValueSaved(value);
                    } else {
                        Log.w(TAG, "Empty gateway not saved");
                    }
                })
                .setNegativeButton("Cancel", null)
                .show();
    }

    private void onValueSaved(String value) {
        FilterListServiceFactory factory = FilterListServiceFactory.getInstance();
        
        factory.setRPCGateWay(value);
        setSummary(value);
        Log.d(TAG, "Saved rpc gateway: " + value);
    }
}
