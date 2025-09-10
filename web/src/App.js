"use strict";
var __rest = (this && this.__rest) || function (s, e) {
    var t = {};
    for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
        for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
            if (e.indexOf(p[i]) < 0 && Object.prototype.propertyIsEnumerable.call(s, p[i]))
                t[p[i]] = s[p[i]];
        }
    return t;
};
Object.defineProperty(exports, "__esModule", { value: true });
var react_1 = require("react");
var material_1 = require("@mui/material");
var icons_material_1 = require("@mui/icons-material");
var react_router_dom_1 = require("react-router-dom");
// Import pages
var Dashboard_1 = require("./pages/Dashboard");
var RouterSimulation_1 = require("./pages/RouterSimulation");
var CloudNetworking_1 = require("./pages/CloudNetworking");
var CloudPods_1 = require("./pages/CloudPods");
var AviatrixIntegration_1 = require("./pages/AviatrixIntegration");
var Analytics_1 = require("./pages/Analytics");
var NetworkTopology_1 = require("./pages/NetworkTopology");
var TrafficShaping_1 = require("./pages/TrafficShaping");
var NetworkImpairments_1 = require("./pages/NetworkImpairments");
// Create theme
var theme = (0, material_1.createTheme)({
    palette: {
        mode: 'dark',
        primary: {
            main: '#1976d2',
        },
        secondary: {
            main: '#dc004e',
        },
        background: {
            default: '#0a0a0a',
            paper: '#1a1a1a',
        },
    },
    typography: {
        fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
        h4: {
            fontWeight: 600,
        },
        h6: {
            fontWeight: 500,
        },
    },
    components: {
        MuiPaper: {
            styleOverrides: {
                root: {
                    backgroundImage: 'none',
                },
            },
        },
    },
});
function TabPanel(props) {
    var children = props.children, value = props.value, index = props.index, other = __rest(props, ["children", "value", "index"]);
    return (<div role="tabpanel" hidden={value !== index} id={"simple-tabpanel-".concat(index)} aria-labelledby={"simple-tab-".concat(index)} {...other}>
      {value === index && <material_1.Box sx={{ p: 3 }}>{children}</material_1.Box>}
    </div>);
}
function App() {
    var _a = (0, react_1.useState)(0), currentTab = _a[0], setCurrentTab = _a[1];
    var _b = (0, react_1.useState)(null), anchorEl = _b[0], setAnchorEl = _b[1];
    var _c = (0, react_1.useState)(5), notifications = _c[0], setNotifications = _c[1];
    var _d = (0, react_1.useState)('online'), systemStatus = _d[0], setSystemStatus = _d[1];
    var handleTabChange = function (event, newValue) {
        setCurrentTab(newValue);
    };
    var handleMenuOpen = function (event) {
        setAnchorEl(event.currentTarget);
    };
    var handleMenuClose = function () {
        setAnchorEl(null);
    };
    var tabs = [
        { label: 'Dashboard', icon: <icons_material_1.Dashboard />, component: <Dashboard_1.default /> },
        { label: 'Router Simulation', icon: <icons_material_1.Router />, component: <RouterSimulation_1.default /> },
        { label: 'Cloud Networking', icon: <icons_material_1.Cloud />, component: <CloudNetworking_1.default /> },
        { label: 'CloudPods', icon: <icons_material_1.Storage />, component: <CloudPods_1.default /> },
        { label: 'Aviatrix', icon: <icons_material_1.NetworkCheck />, component: <AviatrixIntegration_1.default /> },
        { label: 'Analytics', icon: <icons_material_1.Analytics />, component: <Analytics_1.default /> },
        { label: 'Topology', icon: <icons_material_1.Timeline />, component: <NetworkTopology_1.default /> },
        { label: 'Traffic Shaping', icon: <icons_material_1.NetworkCheck />, component: <TrafficShaping_1.default /> },
        { label: 'Impairments', icon: <icons_material_1.Settings />, component: <NetworkImpairments_1.default /> },
    ];
    return (<material_1.ThemeProvider theme={theme}>
      <material_1.CssBaseline />
      <react_router_dom_1.BrowserRouter>
        <material_1.Box sx={{ flexGrow: 1 }}>
          <material_1.AppBar position="static" elevation={0}>
            <material_1.Toolbar>
              <icons_material_1.Router sx={{ mr: 2 }}/>
              <material_1.Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                Multi-Protocol Router Simulator
              </material_1.Typography>
              
              <material_1.Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
                <material_1.Chip label={systemStatus} color={systemStatus === 'online' ? 'success' : 'error'} size="small"/>
                
                <material_1.Tooltip title="Notifications">
                  <material_1.IconButton color="inherit">
                    <material_1.Badge badgeContent={notifications} color="error">
                      <icons_material_1.Notifications />
                    </material_1.Badge>
                  </material_1.IconButton>
                </material_1.Tooltip>
                
                <material_1.Tooltip title="Account">
                  <material_1.IconButton color="inherit" onClick={handleMenuOpen}>
                    <icons_material_1.AccountCircle />
                  </material_1.IconButton>
                </material_1.Tooltip>
              </material_1.Box>
            </material_1.Toolbar>
          </material_1.AppBar>

          <material_1.Container maxWidth="xl" sx={{ mt: 2 }}>
            <material_1.Paper elevation={1} sx={{ mb: 2 }}>
              <material_1.Tabs value={currentTab} onChange={handleTabChange} variant="scrollable" scrollButtons="auto" sx={{ borderBottom: 1, borderColor: 'divider' }}>
                {tabs.map(function (tab, index) { return (<material_1.Tab key={index} label={tab.label} icon={tab.icon} iconPosition="start"/>); })}
              </material_1.Tabs>
            </material_1.Paper>

            <material_1.Box sx={{ minHeight: '70vh' }}>
              {tabs.map(function (tab, index) { return (<TabPanel key={index} value={currentTab} index={index}>
                  {tab.component}
                </TabPanel>); })}
            </material_1.Box>
          </material_1.Container>

          <material_1.Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={handleMenuClose}>
            <material_1.MenuItem onClick={handleMenuClose}>Profile</material_1.MenuItem>
            <material_1.MenuItem onClick={handleMenuClose}>Settings</material_1.MenuItem>
            <material_1.MenuItem onClick={handleMenuClose}>Logout</material_1.MenuItem>
          </material_1.Menu>
        </material_1.Box>
      </react_router_dom_1.BrowserRouter>
    </material_1.ThemeProvider>);
}
exports.default = App;
