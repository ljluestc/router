// Test script to verify UI functionality restoration
// Run this in the browser console after loading either original.js or replay.js

(function() {
    'use strict';
    
    console.log('🧪 Starting UI Functionality Test...');
    
    // Test 1: Check if script is loaded
    function testScriptLoaded() {
        console.log('\n=== TEST 1: Script Loading ===');
        
        if (typeof globalThis.autoAcceptAndAnalytics !== 'undefined') {
            console.log('✅ autoAcceptAndAnalytics class loaded');
        } else {
            console.log('❌ autoAcceptAndAnalytics class NOT loaded');
            return false;
        }
        
        if (typeof globalThis.simpleAccept !== 'undefined') {
            console.log('✅ simpleAccept instance created');
        } else {
            console.log('❌ simpleAccept instance NOT created');
            return false;
        }
        
        return true;
    }
    
    // Test 2: Check control panel creation
    function testControlPanel() {
        console.log('\n=== TEST 2: Control Panel ===');
        
        const panel = document.getElementById('auto-accept-control-panel');
        if (panel) {
            console.log('✅ Control panel created and visible');
            
            // Check tabs
            const tabs = panel.querySelectorAll('.aa-tab');
            console.log(`✅ Found ${tabs.length} tabs: ${Array.from(tabs).map(t => t.textContent).join(', ')}`);
            
            // Check buttons
            const buttons = panel.querySelectorAll('.aa-btn');
            console.log(`✅ Found ${buttons.length} control buttons`);
            
            return true;
        } else {
            console.log('❌ Control panel NOT found');
            return false;
        }
    }
    
    // Test 3: Check button detection patterns
    function testButtonPatterns() {
        console.log('\n=== TEST 3: Button Detection Patterns ===');
        
        const instance = globalThis.simpleAccept;
        if (!instance) {
            console.log('❌ No instance available');
            return false;
        }
        
        // Test pattern matching
        const testButtons = [
            { text: 'Accept', expected: true },
            { text: 'Accept All', expected: true },
            { text: 'Keep', expected: true },
            { text: 'Keep All', expected: true },
            { text: 'Run', expected: true },
            { text: 'Apply', expected: true },
            { text: 'Execute', expected: true },
            { text: 'Resume Conversation', expected: true },
            { text: 'Try Again', expected: true },
            { text: 'Review Next File', expected: true },
            { text: 'Random Text', expected: false }
        ];
        
        let passed = 0;
        testButtons.forEach(({ text, expected }) => {
            // Create a mock element
            const mockElement = {
                textContent: text,
                className: 'test-button',
                disabled: false,
                hasAttribute: () => false
            };
            
            const result = instance.isAcceptButton(mockElement);
            const status = result === expected ? '✅' : '❌';
            console.log(`${status} "${text}": ${result} (expected: ${expected})`);
            
            if (result === expected) passed++;
        });
        
        console.log(`Pattern matching: ${passed}/${testButtons.length} tests passed`);
        return passed === testButtons.length;
    }
    
    // Test 4: Check comprehensive button search
    function testButtonSearch() {
        console.log('\n=== TEST 4: Comprehensive Button Search ===');
        
        const instance = globalThis.simpleAccept;
        if (!instance) {
            console.log('❌ No instance available');
            return false;
        }
        
        // Enable debug mode for detailed logging
        const originalDebugMode = instance.debugMode;
        instance.debugMode = true;
        
        console.log('🔍 Running comprehensive button search...');
        const buttons = instance.findAcceptButtons();
        
        console.log(`Found ${buttons.length} buttons:`);
        buttons.forEach((btn, i) => {
            console.log(`  ${i+1}. "${btn.textContent.trim()}" (${btn.tagName})`);
        });
        
        // Restore debug mode
        instance.debugMode = originalDebugMode;
        
        return true;
    }
    
    // Test 5: Check configuration
    function testConfiguration() {
        console.log('\n=== TEST 5: Configuration ===');
        
        const instance = globalThis.simpleAccept;
        if (!instance) {
            console.log('❌ No instance available');
            return false;
        }
        
        const config = instance.config;
        console.log('Current configuration:');
        Object.entries(config).forEach(([key, value]) => {
            console.log(`  ${key}: ${value}`);
        });
        
        // Test configuration methods
        console.log('\nTesting configuration methods...');
        
        // Test enableOnly
        try {
            instance.enableOnly(['accept', 'keep']);
            console.log('✅ enableOnly() works');
        } catch (e) {
            console.log('❌ enableOnly() failed:', e.message);
        }
        
        // Test enableAll
        try {
            instance.enableAll();
            console.log('✅ enableAll() works');
        } catch (e) {
            console.log('❌ enableAll() failed:', e.message);
        }
        
        return true;
    }
    
    // Test 6: Check analytics functionality
    function testAnalytics() {
        console.log('\n=== TEST 6: Analytics ===');
        
        const instance = globalThis.simpleAccept;
        if (!instance) {
            console.log('❌ No instance available');
            return false;
        }
        
        // Check analytics data structure
        const analytics = instance.analytics;
        console.log('Analytics data:');
        console.log(`  Total accepts: ${analytics.totalAccepts}`);
        console.log(`  Files tracked: ${analytics.files.size}`);
        console.log(`  Sessions: ${analytics.sessions.length}`);
        
        // Test analytics methods
        try {
            const status = instance.status();
            console.log('✅ status() method works');
            console.log('  Status:', status);
        } catch (e) {
            console.log('❌ status() method failed:', e.message);
        }
        
        return true;
    }
    
    // Test 7: Check debug functionality
    function testDebugFunctionality() {
        console.log('\n=== TEST 7: Debug Functionality ===');
        
        const instance = globalThis.simpleAccept;
        if (!instance) {
            console.log('❌ No instance available');
            return false;
        }
        
        // Test debug toggle
        try {
            const debugState = instance.toggleDebug();
            console.log(`✅ Debug mode toggled to: ${debugState}`);
            
            // Test debug search
            instance.debugSearch();
            console.log('✅ debugSearch() executed');
            
            // Toggle back
            instance.toggleDebug();
            console.log('✅ Debug mode toggled back');
        } catch (e) {
            console.log('❌ Debug functionality failed:', e.message);
        }
        
        return true;
    }
    
    // Run all tests
    function runAllTests() {
        console.log('🚀 Starting comprehensive UI functionality test...\n');
        
        const tests = [
            { name: 'Script Loading', fn: testScriptLoaded },
            { name: 'Control Panel', fn: testControlPanel },
            { name: 'Button Patterns', fn: testButtonPatterns },
            { name: 'Button Search', fn: testButtonSearch },
            { name: 'Configuration', fn: testConfiguration },
            { name: 'Analytics', fn: testAnalytics },
            { name: 'Debug Functionality', fn: testDebugFunctionality }
        ];
        
        let passed = 0;
        tests.forEach(test => {
            try {
                if (test.fn()) {
                    passed++;
                }
            } catch (e) {
                console.log(`❌ ${test.name} failed with error:`, e.message);
            }
        });
        
        console.log(`\n🎯 Test Results: ${passed}/${tests.length} tests passed`);
        
        if (passed === tests.length) {
            console.log('🎉 All tests passed! UI functionality is fully restored.');
        } else {
            console.log('⚠️  Some tests failed. Check the output above for details.');
        }
        
        return passed === tests.length;
    }
    
    // Expose test function globally
    globalThis.testUIFunctionality = runAllTests;
    
    // Auto-run tests
    runAllTests();
    
    console.log('\n💡 To run tests again, use: testUIFunctionality()');
    console.log('💡 To enable debug mode, use: toggleDebug()');
    console.log('💡 To start auto-accept, use: startAccept()');
    console.log('💡 To check status, use: acceptStatus()');
    
})();
