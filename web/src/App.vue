<template>
  <div id="app" class="app-container">
    <!-- Navigation Header -->
    <AppHeader />
    
    <!-- Main Content -->
    <main class="main-content">
      <router-view v-slot="{ Component, route }">
        <transition name="fade" mode="out-in">
          <component :is="Component" :key="route.path" />
        </transition>
      </router-view>
    </main>
    
    <!-- Footer -->
    <AppFooter />
    
    <!-- Global Notifications -->
    <NotificationContainer />
    
    <!-- Back to Top Button -->
    <el-backtop :right="40" :bottom="40" />
  </div>
</template>

<script setup lang="ts">
import { onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useAppStore } from '@/stores/app'
import { useWebSocketStore } from '@/stores/websocket'
import AppHeader from '@/components/layout/AppHeader.vue'
import AppFooter from '@/components/layout/AppFooter.vue'
import NotificationContainer from '@/components/common/NotificationContainer.vue'

const router = useRouter()
const appStore = useAppStore()
const wsStore = useWebSocketStore()

// Initialize app
onMounted(async () => {
  try {
    // Initialize app store
    await appStore.initialize()
    
    // Connect to WebSocket
    wsStore.connect()
    
    // Set up global error handling
    window.addEventListener('unhandledrejection', handleGlobalError)
    window.addEventListener('error', handleGlobalError)
    
    console.log('Router Simulator initialized successfully')
  } catch (error) {
    console.error('Failed to initialize app:', error)
  }
})

onUnmounted(() => {
  // Cleanup
  wsStore.disconnect()
  window.removeEventListener('unhandledrejection', handleGlobalError)
  window.removeEventListener('error', handleGlobalError)
})

function handleGlobalError(event: any) {
  console.error('Global error:', event)
  // You can add error reporting here
}
</script>

<style lang="scss">
.app-container {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  background: var(--el-bg-color);
}

.main-content {
  flex: 1;
  padding-top: 60px; // Account for fixed header
}

// Global transitions
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.3s ease;
}

.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}

// Global scrollbar styling
::-webkit-scrollbar {
  width: 8px;
  height: 8px;
}

::-webkit-scrollbar-track {
  background: var(--el-fill-color-lighter);
  border-radius: 4px;
}

::-webkit-scrollbar-thumb {
  background: var(--el-fill-color-dark);
  border-radius: 4px;
  
  &:hover {
    background: var(--el-fill-color-darker);
  }
}

// Dark mode support
.dark {
  .app-container {
    background: var(--el-bg-color-page);
  }
}

// Responsive design
@media (max-width: 768px) {
  .main-content {
    padding-top: 56px; // Smaller header on mobile
  }
}

// Print styles
@media print {
  .app-container {
    background: white !important;
  }
  
  .main-content {
    padding-top: 0;
  }
}
</style>
