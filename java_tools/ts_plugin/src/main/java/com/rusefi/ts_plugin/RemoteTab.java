package com.rusefi.ts_plugin;

import com.rusefi.NamedThreadFactory;
import com.rusefi.SignatureHelper;
import com.rusefi.autoupdate.AutoupdateUtil;
import com.rusefi.io.tcp.ServerSocketReference;
import com.rusefi.io.tcp.TcpIoStream;
import com.rusefi.proxy.client.LocalApplicationProxy;
import com.rusefi.proxy.client.LocalApplicationProxyContextImpl;
import com.rusefi.server.ApplicationRequest;
import com.rusefi.server.ControllerInfo;
import com.rusefi.server.SessionDetails;
import com.rusefi.server.UserDetails;
import com.rusefi.tools.online.HttpUtil;
import com.rusefi.tools.online.ProxyClient;
import com.rusefi.tools.online.PublicSession;
import com.rusefi.ui.AuthTokenPanel;
import com.rusefi.ui.util.URLLabel;
import org.putgemin.VerticalFlowLayout;

import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicReference;

import static com.rusefi.ui.storage.PersistentConfiguration.getConfig;

/**
 * remote ECU access & control
 *
 * @see RemoteTabSandbox
 * @see PluginEntry
 */
public class RemoteTab {
    private static final String APPLICATION_PORT = "application_port";
    public static final String HOWTO_REMOTE_TUNING = "https://github.com/rusefi/rusefi/wiki/HOWTO-Remote-Tuning";
    private final JComponent content = new JPanel(new BorderLayout());

    private final JPanel list = new JPanel(new VerticalFlowLayout());
    private final JTextField oneTimePasswordControl = new JTextField("0") {
        @Override
        public Dimension getPreferredSize() {
            Dimension size = super.getPreferredSize();
            // todo: dynamic calculation of desired with based on String width?
            return new Dimension(100, size.height);
        }
    };


    private final JButton disconnect = new JButton("Disconnect");

    private final Executor listDownloadExecutor = Executors.newSingleThreadExecutor(new NamedThreadFactory("online list downloader"));

    public RemoteTab() {
        JButton refresh = new JButton("Refresh List");
        refresh.addActionListener(e -> requestListDownload());

        disconnect.addActionListener(e -> {
            LocalApplicationProxy localApplicationProxy = RemoteTabController.INSTANCE.getLocalApplicationProxy();
            if (localApplicationProxy != null)
                localApplicationProxy.close();
            RemoteTabController.INSTANCE.setState(RemoteTabController.State.NOT_CONNECTED);
            requestListDownload();
        });


        JTextField applicationPort = new JTextField() {
            @Override
            public Dimension getPreferredSize() {
                Dimension size = super.getPreferredSize();
                // todo: dynamic calculation of desired with based on String width?
                return new Dimension(100, size.height);
            }
        };
        IntegerDocumentFilter.install(applicationPort);
        IntegerDocumentFilter.install(oneTimePasswordControl);
        String portProperty = getLocalPort();
        applicationPort.setText(portProperty);

        JPanel topLines = new JPanel(new VerticalFlowLayout());


        JPanel topPanel = new JPanel(new FlowLayout());
        topPanel.add(refresh);
        topPanel.add(new JLabel("    Local Port: "));
        topPanel.add(applicationPort);
        topPanel.add(new JLabel("   One time password:"));
        topPanel.add(oneTimePasswordControl);

        topLines.add(topPanel);
        topLines.add(new URLLabel(HOWTO_REMOTE_TUNING));

        content.add(topLines, BorderLayout.NORTH);
        content.add(list, BorderLayout.CENTER);
        list.add(new JLabel("Requesting list of ECUs"));

        LocalApplicationProxy currentState = RemoteTabController.INSTANCE.getLocalApplicationProxy();
        if (currentState == null) {
            requestListDownload();
        } else {
            setConnectedStatus(currentState.getApplicationRequest().getTargetUser());
        }
    }

    private String getLocalPort() {
        return getConfig().getRoot().getProperty(APPLICATION_PORT, "29001");
    }

    private void requestListDownload() {
        listDownloadExecutor.execute(() -> {
            List<PublicSession> userDetails;
            try {
                userDetails = ProxyClient.getOnlineApplications(HttpUtil.PROXY_JSON_API_HTTP_PORT);
                SwingUtilities.invokeLater(() -> showList(userDetails));
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }
        });
    }

    private void showList(List<PublicSession> userDetails) {
        if (RemoteTabController.INSTANCE.getState() != RemoteTabController.State.NOT_CONNECTED)
            return;
        list.removeAll();
        if (userDetails.isEmpty()) {
            list.add(new JLabel("No ECUs are broadcasting at the moment :("));
        } else {

            JPanel verticalPanel = new JPanel(new VerticalFlowLayout());
            JScrollPane scroll = new JScrollPane(verticalPanel, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            list.add(scroll);

            for (PublicSession user : userDetails) {
                verticalPanel.add(createSessionControl(user));
            }
        }
        AutoupdateUtil.trueLayout(list);
    }

    private JComponent createSessionControl(PublicSession publicSession) {
        JComponent topLine = new JPanel(new FlowLayout());
        topLine.add(new JLabel(publicSession.getUserDetails().getUserName()));
        ControllerInfo controllerInfo = publicSession.getControllerInfo();
        topLine.add(new JLabel(controllerInfo.getVehicleName() + " " + controllerInfo.getEngineMake() + " " + controllerInfo.getEngineCode()));

        JPanel bottomPanel = new JPanel(new FlowLayout());

        if (publicSession.isUsed()) {
            bottomPanel.add(new JLabel(" Used by " + publicSession.getOwnerName()));
        } else {
            JButton connect = new JButton("Connect to " + publicSession.getUserDetails().getUserName());
            connect.addActionListener(event -> connectToProxy(publicSession, controllerInfo));
            bottomPanel.add(connect);
        }

        JPanel userPanel = new JPanel(new BorderLayout());

        userPanel.add(topLine, BorderLayout.NORTH);
        userPanel.add(new URLLabel(SignatureHelper.getUrl(controllerInfo.getSignature())), BorderLayout.CENTER);
        userPanel.add(bottomPanel, BorderLayout.SOUTH);

        userPanel.setBorder(BorderFactory.createLineBorder(Color.BLACK));

        return userPanel;
    }

    private void connectToProxy(PublicSession publicSession, ControllerInfo controllerInfo) {
        RemoteTabController.INSTANCE.setState(RemoteTabController.State.CONNECTING);
        setStatus("Connecting to " + publicSession.getUserDetails().getUserName());

        LocalApplicationProxy.ConnectionListener connectionListener = localApplicationProxy -> {
            RemoteTabController.INSTANCE.setConnected(localApplicationProxy);
            SwingUtilities.invokeLater(new Runnable() {
                @Override
                public void run() {
                    setConnectedStatus(publicSession.getUserDetails());
                }
            });
        };

        new Thread(() -> {
            runAuthenticator(publicSession, controllerInfo, connectionListener);
        }, "Authenticator").start();
    }

    private void setConnectedStatus(UserDetails userDetails) {
        setStatus("Connected to " + userDetails.getUserName(),
                new JLabel("You can now connect your TunerStudio to IP address localhost and port " + getLocalPort()),
                disconnect);
    }

    private void setStatus(String text, JComponent... extra) {
        list.removeAll();
        list.add(new JLabel(text));
        for (JComponent component : extra)
            list.add(component);
        AutoupdateUtil.trueLayout(list);
    }

    private void runAuthenticator(PublicSession publicSession, ControllerInfo controllerInfo, LocalApplicationProxy.ConnectionListener connectionListener) {
        SessionDetails sessionDetails = new SessionDetails(controllerInfo, AuthTokenPanel.getAuthToken(),
                Integer.parseInt(oneTimePasswordControl.getText()));

        ApplicationRequest applicationRequest = new ApplicationRequest(sessionDetails, publicSession.getUserDetails());

        try {
            AtomicReference<ServerSocketReference> serverHolderAtomicReference = new AtomicReference<>();

            TcpIoStream.DisconnectListener disconnectListener = message -> SwingUtilities.invokeLater(() -> {
                System.out.println("Disconnected " + message);
                setStatus("Disconnected");
                RemoteTabController.INSTANCE.setState(RemoteTabController.State.NOT_CONNECTED);
                ServerSocketReference serverHolder = serverHolderAtomicReference.get();
                if (serverHolder != null)
                    serverHolder.close();
            });

            LocalApplicationProxyContextImpl context = new LocalApplicationProxyContextImpl() {
                @Override
                public int authenticatorPort() {
                    return Integer.parseInt(getLocalPort());
                }
            };
            ServerSocketReference serverHolder = LocalApplicationProxy.startAndRun(
                    context,
                    applicationRequest,
                    HttpUtil.PROXY_JSON_API_HTTP_PORT, disconnectListener, connectionListener);
            serverHolderAtomicReference.set(serverHolder);
        } catch (IOException e) {
            setStatus("IO error: " + e);
        }
    }

    public JComponent getContent() {
        return content;
    }
}
