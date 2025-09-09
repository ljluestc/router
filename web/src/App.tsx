import React from 'react'
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom'
import { Layout } from './components/Layout'
import { Dashboard } from './pages/Dashboard'
import { Analytics } from './pages/Analytics'
import { NetworkTopology } from './pages/NetworkTopology'
import { TrafficShaping } from './pages/TrafficShaping'
import { NetworkImpairments } from './pages/NetworkImpairments'
import { Scenarios } from './pages/Scenarios'
import { Testing } from './pages/Testing'
import { Settings } from './pages/Settings'
import { Documentation } from './pages/Documentation'
import { About } from './pages/About'
import { Aviatrix } from './pages/Aviatrix'
import { CloudPods } from './pages/CloudPods'
import { NotFound } from './pages/NotFound'
import './index.css'

function App() {
  return (
    <Router>
      <Layout>
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/analytics" element={<Analytics />} />
          <Route path="/topology" element={<NetworkTopology />} />
          <Route path="/traffic-shaping" element={<TrafficShaping />} />
          <Route path="/impairments" element={<NetworkImpairments />} />
          <Route path="/scenarios" element={<Scenarios />} />
          <Route path="/testing" element={<Testing />} />
          <Route path="/aviatrix" element={<Aviatrix />} />
          <Route path="/cloudpods" element={<CloudPods />} />
          <Route path="/settings" element={<Settings />} />
          <Route path="/docs" element={<Documentation />} />
          <Route path="/about" element={<About />} />
          <Route path="*" element={<NotFound />} />
        </Routes>
      </Layout>
    </Router>
  )
}

export default App
