// Enhanced Keep Button Detection Test
// Run this in the browser console to test Keep button detection

console.log('=== Enhanced Keep Button Detection Test ===');

// Test 1: Check if the auto-accept script is loaded
if (typeof globalThis.simpleAccept !== 'undefined') {
    console.log('✅ Auto-accept script is loaded');
    
    // Test 2: Enable debug mode for detailed logging
    globalThis.simpleAccept.debugMode = true;
    console.log('✅ Debug mode enabled');
    
    // Test 3: Check current configuration
    console.log('\n=== Current Configuration ===');
    console.log('enableKeep:', globalThis.simpleAccept.config.enableKeep);
    console.log('enableKeepAll:', globalThis.simpleAccept.config.enableKeepAll);
    console.log('enableAccept:', globalThis.simpleAccept.config.enableAccept);
    console.log('enableAcceptAll:', globalThis.simpleAccept.config.enableAcceptAll);
    
    // Test 4: Search for all buttons on the page
    console.log('\n=== Searching for All Buttons ===');
    const allButtons = globalThis.simpleAccept.findAcceptButtons();
    console.log(`Found ${allButtons.length} total buttons:`);
    
    allButtons.forEach((btn, index) => {
        const text = btn.textContent.trim();
        const classes = btn.className;
        const tag = btn.tagName;
        console.log(`  ${index + 1}. "${text}" (${tag}, classes: ${classes})`);
    });
    
    // Test 5: Specific Keep button search
    console.log('\n=== Specific Keep Button Search ===');
    const keepButtons = [];
    const allElements = document.querySelectorAll('*');
    
    allElements.forEach(el => {
        if (el.textContent && el.textContent.toLowerCase().includes('keep')) {
            const text = el.textContent.trim();
            const isVisible = globalThis.simpleAccept.isElementVisible(el);
            const isClickable = globalThis.simpleAccept.isElementClickable(el);
            
            keepButtons.push({
                element: el,
                text: text,
                visible: isVisible,
                clickable: isClickable,
                classes: el.className,
                tag: el.tagName
            });
        }
    });
    
    console.log(`Found ${keepButtons.length} elements containing "keep":`);
    keepButtons.forEach((btn, index) => {
        console.log(`  ${index + 1}. "${btn.text}" (${btn.tag})`);
        console.log(`     Visible: ${btn.visible}, Clickable: ${btn.clickable}`);
        console.log(`     Classes: ${btn.classes}`);
    });
    
    // Test 6: Test Keep button detection logic
    console.log('\n=== Testing Keep Button Detection Logic ===');
    const testElements = keepButtons.map(btn => btn.element);
    const detectedKeepButtons = testElements.filter(el => 
        globalThis.simpleAccept.isAcceptButton(el)
    );
    
    console.log(`Detection logic found ${detectedKeepButtons.length} valid Keep buttons:`);
    detectedKeepButtons.forEach((btn, index) => {
        console.log(`  ${index + 1}. "${btn.textContent.trim()}"`);
    });
    
    // Test 7: Manual search in conversation area
    console.log('\n=== Searching Conversation Area ===');
    const conversationsDiv = document.querySelector('div.conversations');
    if (conversationsDiv) {
        console.log('Found conversations div, searching for Keep buttons...');
        const conversationKeepButtons = globalThis.simpleAccept.findKeepButtonsInElement(conversationsDiv);
        console.log(`Found ${conversationKeepButtons.length} Keep buttons in conversation area:`);
        conversationKeepButtons.forEach((btn, index) => {
            console.log(`  ${index + 1}. "${btn.textContent.trim()}"`);
        });
    } else {
        console.log('No conversations div found');
    }
    
    // Test 8: Test button clicking
    if (detectedKeepButtons.length > 0) {
        console.log('\n=== Testing Keep Button Clicking ===');
        console.log('Clicking the first detected Keep button...');
        
        const firstButton = detectedKeepButtons[0];
        const success = globalThis.simpleAccept.clickElement(firstButton);
        
        if (success) {
            console.log('✅ Keep button clicked successfully!');
        } else {
            console.log('❌ Failed to click Keep button');
        }
    } else {
        console.log('No Keep buttons detected for clicking test');
    }
    
    // Test 9: Show control panel
    console.log('\n=== Control Panel ===');
    console.log('Opening control panel for manual inspection...');
    globalThis.simpleAccept.showControlPanel();
    
} else {
    console.log('❌ Auto-accept script not found. Make sure the script is loaded.');
}

console.log('\n=== Test Complete ===');
console.log('Commands to try:');
console.log('1. enableDebug() - Enable detailed logging');
console.log('2. debugAccept() - Manual debug search');
console.log('3. startAccept() - Start auto-clicking');
console.log('4. showAnalytics() - View analytics panel');
console.log('5. validateData() - Check data consistency');
