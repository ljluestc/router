import React, { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { ConfigProvider } from 'element-plus';
import zhCn from 'element-plus/dist/locale/zh-cn';
import en from 'element-plus/dist/locale/en';

import Layout from './components/Layout';
import Dashboard from './pages/Dashboard';
import NetworkTopology from './pages/NetworkTopology';
import TrafficShaping from './pages/TrafficShaping';
import NetworkImpairments from './pages/NetworkImpairments';
import Analytics from './pages/Analytics';
import Aviatrix from './pages/Aviatrix';
import CloudPods from './pages/CloudPods';
import Scenarios from './pages/Scenarios';
import Testing from './pages/Testing';
import Settings from './pages/Settings';
import Documentation from './pages/Documentation';
import About from './pages/About';
import NotFound from './pages/NotFound';

import './index.css';

const App: React.FC = () => {
  const [locale, setLocale] = useState(en);
  const [theme, setTheme] = useState<'light' | 'dark'>('light');

  useEffect(() => {
    // Load saved preferences
    const savedLocale = localStorage.getItem('locale') || 'en';
    const savedTheme = localStorage.getItem('theme') || 'light';
    
    setLocale(savedLocale === 'zh' ? zhCn : en);
    setTheme(savedTheme as 'light' | 'dark');
  }, []);

  const handleLocaleChange = (newLocale: string) => {
    setLocale(newLocale === 'zh' ? zhCn : en);
    localStorage.setItem('locale', newLocale);
  };

  const handleThemeChange = (newTheme: 'light' | 'dark') => {
    setTheme(newTheme);
    localStorage.setItem('theme', newTheme);
  };

  return (
    <ConfigProvider locale={locale}>
      <div className={`app ${theme}`}>
        <Router>
          <Layout 
            onLocaleChange={handleLocaleChange}
            onThemeChange={handleThemeChange}
            theme={theme}
          >
            <Routes>
              <Route path="/" element={<Navigate to="/dashboard" replace />} />
              <Route path="/dashboard" element={<Dashboard />} />
              <Route path="/network-topology" element={<NetworkTopology />} />
              <Route path="/traffic-shaping" element={<TrafficShaping />} />
              <Route path="/network-impairments" element={<NetworkImpairments />} />
              <Route path="/analytics" element={<Analytics />} />
              <Route path="/aviatrix" element={<Aviatrix />} />
              <Route path="/cloudpods" element={<CloudPods />} />
              <Route path="/scenarios" element={<Scenarios />} />
              <Route path="/testing" element={<Testing />} />
              <Route path="/settings" element={<Settings />} />
              <Route path="/documentation" element={<Documentation />} />
              <Route path="/about" element={<About />} />
              <Route path="*" element={<NotFound />} />
            </Routes>
          </Layout>
        </Router>
      </div>
    </ConfigProvider>
  );
};

export default App;
