import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import { createRouter, createWebHistory } from 'vue-router'
import { createPinia } from 'pinia'
import App from '@/App.vue'
import ElementPlus from 'element-plus'

// Mock components
vi.mock('@/components/layout/AppHeader.vue', () => ({
  default: {
    name: 'AppHeader',
    template: '<div data-testid="app-header">Header</div>'
  }
}))

vi.mock('@/components/layout/AppFooter.vue', () => ({
  default: {
    name: 'AppFooter',
    template: '<div data-testid="app-footer">Footer</div>'
  }
}))

vi.mock('@/components/common/NotificationContainer.vue', () => ({
  default: {
    name: 'NotificationContainer',
    template: '<div data-testid="notification-container">Notifications</div>'
  }
}))

// Mock stores
vi.mock('@/stores/app', () => ({
  useAppStore: () => ({
    initialize: vi.fn().mockResolvedValue(undefined)
  })
}))

vi.mock('@/stores/websocket', () => ({
  useWebSocketStore: () => ({
    connect: vi.fn(),
    disconnect: vi.fn()
  })
}))

describe('App.vue', () => {
  let router: any
  let pinia: any

  beforeEach(() => {
    // Create router
    router = createRouter({
      history: createWebHistory(),
      routes: [
        {
          path: '/',
          name: 'home',
          component: { template: '<div data-testid="home-page">Home</div>' }
        }
      ]
    })

    // Create pinia
    pinia = createPinia()
  })

  it('renders correctly', () => {
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    expect(wrapper.find('[data-testid="app-header"]').exists()).toBe(true)
    expect(wrapper.find('[data-testid="app-footer"]').exists()).toBe(true)
    expect(wrapper.find('[data-testid="notification-container"]').exists()).toBe(true)
  })

  it('has correct structure', () => {
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    expect(wrapper.find('.app-container').exists()).toBe(true)
    expect(wrapper.find('.main-content').exists()).toBe(true)
  })

  it('handles route transitions', async () => {
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    await router.push('/')
    await wrapper.vm.$nextTick()

    expect(wrapper.find('[data-testid="home-page"]').exists()).toBe(true)
  })

  it('applies app-ready class after mount', async () => {
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    await wrapper.vm.$nextTick()

    expect(document.body.classList.contains('app-ready')).toBe(true)
  })

  it('handles global errors', () => {
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
    
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    // Simulate global error
    const errorEvent = new ErrorEvent('error', {
      message: 'Test error',
      filename: 'test.js',
      lineno: 1,
      colno: 1,
      error: new Error('Test error')
    })

    window.dispatchEvent(errorEvent)

    expect(consoleSpy).toHaveBeenCalled()
    
    consoleSpy.mockRestore()
  })

  it('handles unhandled promise rejections', () => {
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
    
    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    // Simulate unhandled promise rejection
    const rejectionEvent = new PromiseRejectionEvent('unhandledrejection', {
      promise: Promise.reject(new Error('Test rejection')),
      reason: new Error('Test rejection')
    })

    window.dispatchEvent(rejectionEvent)

    expect(consoleSpy).toHaveBeenCalled()
    
    consoleSpy.mockRestore()
  })

  it('initializes stores on mount', async () => {
    const mockAppStore = {
      initialize: vi.fn().mockResolvedValue(undefined)
    }
    const mockWsStore = {
      connect: vi.fn(),
      disconnect: vi.fn()
    }

    vi.mocked(require('@/stores/app').useAppStore).mockReturnValue(mockAppStore)
    vi.mocked(require('@/stores/websocket').useWebSocketStore).mockReturnValue(mockWsStore)

    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    await wrapper.vm.$nextTick()

    expect(mockAppStore.initialize).toHaveBeenCalled()
    expect(mockWsStore.connect).toHaveBeenCalled()
  })

  it('cleans up on unmount', async () => {
    const mockWsStore = {
      connect: vi.fn(),
      disconnect: vi.fn()
    }

    vi.mocked(require('@/stores/websocket').useWebSocketStore).mockReturnValue(mockWsStore)

    const wrapper = mount(App, {
      global: {
        plugins: [router, pinia, ElementPlus]
      }
    })

    await wrapper.unmount()

    expect(mockWsStore.disconnect).toHaveBeenCalled()
  })
})
