// Simplified Cursor Auto-Accept Script with File Analytics & ROI Tracking
// Created by @ivalsaraj (https://linkedin.com/in/ivalsaraj)
// GitHub: https://github.com/ivalsaraj/cursor-auto-accept-full-agentic-mode
(function() {
    'use strict';
    
    if (typeof globalThis.autoAcceptAndAnalytics === 'undefined') {
        class autoAcceptAndAnalytics {
            constructor(interval = 2000) {
                this.interval = interval;
                this.isRunning = false;
                this.monitorInterval = null;
                this.totalClicks = 0;
                this.controlPanel = null;
                this.isDragging = false;
                this.dragOffset = { x: 0, y: 0 };
                this.currentTab = 'main'; // 'main', 'analytics', or 'roi'
                this.loggedMessages = new Set(); // Track logged messages to prevent duplicates
                this.debugMode = false; // Control debug logging
                
                // IDE Detection
                this.ideType = this.detectIDE();
                this.log(`Detected IDE: ${this.ideType}`);
                
                // File analytics tracking
                this.analytics = {
                    files: new Map(), // filename -> { acceptCount, lastAccepted, addedLines, deletedLines }
                    sessions: [], // track session data
                    totalAccepts: 0,
                    sessionStart: new Date()
                };
                
                // ROI Time tracking - Complete workflow measurement
                this.roiTracking = {
                    totalTimeSaved: 0, // in milliseconds
                    codeGenerationSessions: [],
                    // Complete workflow times (user prompt → cursor completion)
                    averageCompleteWorkflow: 30000, // 30 seconds: user watches cursor generate + manually accept
                    averageAutomatedWorkflow: 100, // ~100ms: script detects and clicks instantly
                    // Manual workflow breakdown:
                    // - User sends prompt: 0s (same for both)
                    // - Cursor generates: 10-20s (same for both) 
                    // - User watches generation: 5-15s (eliminated by script)
                    // - User finds and clicks button: 2-3s (eliminated by script)
                    // - Context switching: 1-2s (eliminated by script)
                    currentWorkflowStart: null,
                    currentSessionButtons: 0,
                    workflowSessions: [] // Track individual workflow times
                };
                
                // Configuration for button types
                this.config = {
                    enableAcceptAll: true,
                    enableAccept: true,
                    enableReviewNextFile: true,    // New: Review next file button
                    enableKeepAll: true,           // New: Keep all button
                    enableKeep: true,              // New: Keep button
                    enableRun: true,
                    enableRunCommand: true,
                    enableApply: true,
                    enableExecute: true,
                    enableResume: true,
                    enableConnectionResume: true,  // New: Resume button in connection failure dropdown
                    enableTryAgain: true           // New: Try again button in connection failure dropdown
                };
                
                // Load persisted data
                this.loadFromStorage();
                
                this.createControlPanel();
                this.log('autoAcceptAndAnalytics initialized with file analytics and ROI tracking');
            }
            
            // Persistence methods
            saveToStorage() {
                try {
                    const data = {
                        analytics: {
                            files: Array.from(this.analytics.files.entries()),
                            sessions: this.analytics.sessions,
                            totalAccepts: this.analytics.totalAccepts,
                            sessionStart: this.analytics.sessionStart
                        },
                        roiTracking: this.roiTracking,
                        config: this.config,
                        totalClicks: this.totalClicks,
                        savedAt: new Date()
                    };
                    localStorage.setItem('cursor-auto-accept-data', JSON.stringify(data));
                } catch (error) {
                    console.warn('Failed to save to localStorage:', error);
                }
            }
            
            loadFromStorage() {
                try {
                    const saved = localStorage.getItem('cursor-auto-accept-data');
                    if (saved) {
                        const data = JSON.parse(saved);
                        
                        // Restore analytics
                        if (data.analytics) {
                            this.analytics.files = new Map(data.analytics.files || []);
                            this.analytics.sessions = data.analytics.sessions || [];
                            this.analytics.totalAccepts = data.analytics.totalAccepts || 0;
                            this.analytics.sessionStart = data.analytics.sessionStart ? new Date(data.analytics.sessionStart) : new Date();
                        }
                        
                        // Restore ROI tracking
                        if (data.roiTracking) {
                            this.roiTracking = { ...this.roiTracking, ...data.roiTracking };
                        }
                        
                        // Restore config
                        if (data.config) {
                            this.config = { ...this.config, ...data.config };
                        }
                        
                        // Restore click count
                        if (data.totalClicks) {
                            this.totalClicks = data.totalClicks;
                        }
                        
                        console.log('Data loaded from localStorage');
                    }
                } catch (error) {
                    console.warn('Failed to load from localStorage:', error);
                }
            }
            
            clearStorage() {
                try {
                    localStorage.removeItem('cursor-auto-accept-data');
                    console.log('Storage cleared');
                    
                    // Reset current session data too
                    this.analytics.files.clear();
                    this.analytics.sessions = [];
                    this.analytics.totalAccepts = 0;
                    this.analytics.sessionStart = new Date();
                    this.roiTracking.totalTimeSaved = 0;
                    this.totalClicks = 0;
                    
                    // Update UI
                    this.updateAnalyticsContent();
                    this.updateMainFooter();
                    this.updatePanelStatus();
                    
                    this.logToPanel('🗑️ All data cleared (storage + session)', 'warning');
                } catch (error) {
                    console.warn('Failed to clear localStorage:', error);
                }
            }
            
            validateData() {
                console.log('=== DATA VALIDATION ===');
                console.log('Session Info:');
                console.log(`  Session Start: ${this.analytics.sessionStart}`);
                console.log(`  Total Accepts: ${this.analytics.totalAccepts}`);
                console.log(`  Total Clicks: ${this.totalClicks}`);
                console.log(`  Total Time Saved: ${this.formatTimeDuration(this.roiTracking.totalTimeSaved)}`);
                
                console.log('\nFiles Tracked:');
                this.analytics.files.forEach((data, filename) => {
                    console.log(`  ${filename}:`);
                    console.log(`    Accept Count: ${data.acceptCount}`);
                    console.log(`    Total Added: +${data.totalAdded}`);
                    console.log(`    Total Deleted: -${data.totalDeleted}`);
                    console.log(`    Last Accepted: ${data.lastAccepted}`);
                });
                
                console.log('\nRecent Sessions:');
                this.analytics.sessions.slice(-5).forEach((session, i) => {
                    console.log(`  ${i+1}. ${session.filename} (+${session.addedLines}/-${session.deletedLines}) [${session.buttonType}] at ${session.timestamp}`);
                });
                
                console.log('\nLocalStorage Check:');
                try {
                    const saved = localStorage.getItem('cursor-auto-accept-data');
                    if (saved) {
                        const data = JSON.parse(saved);
                        console.log('  Storage exists, saved at:', data.savedAt);
                        console.log('  Storage analytics total accepts:', data.analytics?.totalAccepts || 0);
                        console.log('  Storage total clicks:', data.totalClicks || 0);
                    } else {
                        console.log('  No data in localStorage');
                    }
                } catch (error) {
                    console.log('  Error reading localStorage:', error);
                }
                
                console.log('=== END VALIDATION ===');
                return {
                    currentSession: {
                        totalAccepts: this.analytics.totalAccepts,
                        totalClicks: this.totalClicks,
                        timeSaved: this.roiTracking.totalTimeSaved,
                        filesCount: this.analytics.files.size
                    },
                    isDataConsistent: this.analytics.totalAccepts === this.analytics.sessions.length
                };
            }
            
            toggleDebug() {
                this.debugMode = !this.debugMode;
                console.log(`Debug mode ${this.debugMode ? 'enabled' : 'disabled'}`);
                this.logToPanel(`Debug mode ${this.debugMode ? 'ON' : 'OFF'}`, 'info');
                return this.debugMode;
            }

            // Detect which IDE we're running in
            detectIDE() {
                try {
                    // Check for Windsurf-specific elements and classes
                    const windsurfIndicators = [
                        'bg-ide-editor-background',
                        'bg-ide-button-background', 
                        'text-ide-button-color',
                        'hover:bg-ide-button-hover-background'
                    ];
                    
                    // Check for Cursor-specific elements
                    const cursorIndicators = [
                        'div.full-input-box',
                        '.composer-code-block-container',
                        '.anysphere-text-button',
                        '.anysphere-secondary-button'
                    ];
                    
                    // Look for Windsurf indicators in the DOM
                    let windsurfScore = 0;
                    let cursorScore = 0;
                    
                    // Check for Windsurf class patterns in stylesheets and elements
                    windsurfIndicators.forEach(indicator => {
                        if (document.querySelector(`[class*="${indicator}"]`) || 
                            document.body.innerHTML.includes(indicator)) {
                            windsurfScore++;
                        }
                    });
                    
                    // Check for Cursor-specific selectors
                    cursorIndicators.forEach(selector => {
                        if (document.querySelector(selector)) {
                            cursorScore++;
                        }
                    });
                    
                    // Check URL and title for additional clues
                    const url = window.location.href.toLowerCase();
                    const title = document.title.toLowerCase();
                    
                    if (url.includes('windsurf') || title.includes('windsurf')) {
                        windsurfScore += 2;
                    }
                    
                    if (url.includes('cursor') || title.includes('cursor')) {
                        cursorScore += 2;
                    }
                    
                    // Determine IDE based on scores
                    if (windsurfScore > cursorScore) {
                        return 'windsurf';
                    } else if (cursorScore > 0) {
                        return 'cursor';
                    } else {
                        // Default to cursor if uncertain
                        return 'cursor';
                    }
                    
                } catch (error) {
                    this.log(`IDE detection error: ${error.message}`);
                    return 'cursor'; // Default fallback
                }
            }
            
            calibrateWorkflowTimes(manualWorkflowSeconds, automatedWorkflowMs = 100) {
                const oldManual = this.roiTracking.averageCompleteWorkflow;
                const oldAuto = this.roiTracking.averageAutomatedWorkflow;
                
                this.roiTracking.averageCompleteWorkflow = manualWorkflowSeconds * 1000;
                this.roiTracking.averageAutomatedWorkflow = automatedWorkflowMs;
                
                console.log(`Workflow times updated:`);
                console.log(`  Manual: ${oldManual/1000}s → ${manualWorkflowSeconds}s`);
                console.log(`  Automated: ${oldAuto}ms → ${automatedWorkflowMs}ms`);
                
                // Recalculate all existing workflow sessions
                this.roiTracking.totalTimeSaved = 0;
                this.roiTracking.workflowSessions.forEach(session => {
                    const timeSaved = this.roiTracking.averageCompleteWorkflow - this.roiTracking.averageAutomatedWorkflow;
                    this.roiTracking.totalTimeSaved += timeSaved;
                    session.timeSaved = timeSaved;
                });
                
                this.saveToStorage();
                this.updateAnalyticsContent();
                this.updateMainFooter();
                
                this.logToPanel(`Workflow calibrated: ${manualWorkflowSeconds}s manual`, 'info');
                return { manual: manualWorkflowSeconds, automated: automatedWorkflowMs };
            }
            
            // ROI Tracking Methods
            startCodeGenSession() {
                this.roiTracking.currentSessionStart = new Date();
                this.roiTracking.currentSessionButtons = 0;
            }
            
            endCodeGenSession() {
                if (this.roiTracking.currentSessionStart) {
                    const sessionDuration = new Date() - this.roiTracking.currentSessionStart;
                    this.roiTracking.codeGenerationSessions.push({
                        start: this.roiTracking.currentSessionStart,
                        duration: sessionDuration,
                        buttonsClicked: this.roiTracking.currentSessionButtons,
                        timeSaved: this.roiTracking.currentSessionButtons * this.roiTracking.manualClickTime
                    });
                    this.roiTracking.currentSessionStart = null;
                }
            }
            
            calculateTimeSaved(buttonType) {
                // Calculate time saved based on complete workflow automation
                // Manual workflow: User watches generation + finds button + clicks + context switch
                // Automated workflow: Script detects and clicks instantly while user can focus on coding
                
                const workflowTimeSavings = {
                    'accept all': this.roiTracking.averageCompleteWorkflow + 5000, // extra time to review all changes
                    'accept': this.roiTracking.averageCompleteWorkflow,
                    'review next file': this.roiTracking.averageCompleteWorkflow + 2000, // time to navigate and review
                    'keep all': this.roiTracking.averageCompleteWorkflow + 5000, // extra time to review all changes
                    'keep': this.roiTracking.averageCompleteWorkflow,
                    'run': this.roiTracking.averageCompleteWorkflow + 2000, // extra caution for running commands
                    'execute': this.roiTracking.averageCompleteWorkflow + 2000,
                    'apply': this.roiTracking.averageCompleteWorkflow,
                    'resume': this.roiTracking.averageCompleteWorkflow + 3000, // time saved by auto-resuming conversation
                    'connection-resume': this.roiTracking.averageCompleteWorkflow + 4000, // extra time for connection issues
                    'try again': this.roiTracking.averageCompleteWorkflow + 3000 // time saved by auto-retrying
                };
                
                const manualTime = workflowTimeSavings[buttonType.toLowerCase()] || this.roiTracking.averageCompleteWorkflow;
                const automatedTime = this.roiTracking.averageAutomatedWorkflow;
                const timeSaved = manualTime - automatedTime;
                
                this.roiTracking.totalTimeSaved += timeSaved;
                this.roiTracking.currentSessionButtons++;
                
                // Track this workflow session
                this.roiTracking.workflowSessions.push({
                    timestamp: new Date(),
                    buttonType: buttonType,
                    manualTime: manualTime,
                    automatedTime: automatedTime,
                    timeSaved: timeSaved
                });
                
                // Save to storage after each update
                this.saveToStorage();
                
                return timeSaved;
            }
            
            formatTimeDuration(milliseconds) {
                if (!milliseconds || isNaN(milliseconds) || milliseconds <= 0) return '0s';
                
                const totalSeconds = Math.floor(milliseconds / 1000);
                const hours = Math.floor(totalSeconds / 3600);
                const minutes = Math.floor((totalSeconds % 3600) / 60);
                const seconds = totalSeconds % 60;
                
                if (hours > 0) {
                    return `${hours}h ${minutes}m ${seconds}s`;
                } else if (minutes > 0) {
                    return `${minutes}m ${seconds}s`;
                } else {
                    return `${seconds}s`;
                }
            }
            
            // Extract file info from code blocks when button is clicked
            extractFileInfo(button) {
                try {
                    if (this.debugMode) {
                        this.log('=== DEBUG: extractFileInfo called ===');
                        this.log(`Button text: "${button.textContent.trim()}"`);
                        this.log(`Button classes: ${button.className}`);
                    }

                    // New approach: Find the latest diff block in conversations div
                    const conversationsDiv = document.querySelector('div.conversations');
                    if (!conversationsDiv) {
                        if (this.debugMode) this.log('DEBUG: No conversations div found');
                        return null;
                    }

                    // Find all message bubbles with data-message-index, sorted by index (latest first)
                    const messageBubbles = Array.from(conversationsDiv.querySelectorAll('[data-message-index]'))
                        .sort((a, b) => {
                            const indexA = parseInt(a.getAttribute('data-message-index'));
                            const indexB = parseInt(b.getAttribute('data-message-index'));
                            return indexB - indexA; // Descending order (latest first)
                        });

                    if (this.debugMode) {
                        this.log(`DEBUG: Found ${messageBubbles.length} message bubbles`);
                        if (messageBubbles.length > 0) {
                            const latestIndex = messageBubbles[0].getAttribute('data-message-index');
                            this.log(`DEBUG: Latest message index: ${latestIndex}`);
                        }
                    }

                    // Look for diff blocks in the latest few messages
                    for (let i = 0; i < Math.min(5, messageBubbles.length); i++) {
                        const bubble = messageBubbles[i];
                        const messageIndex = bubble.getAttribute('data-message-index');
                        
                        if (this.debugMode) {
                            this.log(`DEBUG: Checking message ${messageIndex}`);
                        }

                        // Look for code block containers within this message
                        const codeBlocks = bubble.querySelectorAll('.composer-code-block-container, .composer-tool-former-message, .composer-diff-block');
                        
                        if (this.debugMode && codeBlocks.length > 0) {
                            this.log(`DEBUG: Found ${codeBlocks.length} code blocks in message ${messageIndex}`);
                        }

                        for (const block of codeBlocks) {
                            const fileInfo = this.extractFileInfoFromBlock(block);
                            if (fileInfo) {
                                if (this.debugMode) {
                                    this.log(`DEBUG: Successfully extracted file info: ${JSON.stringify(fileInfo)}`);
                                }
                                return fileInfo;
                            }
                        }
                    }

                    if (this.debugMode) {
                        this.log('DEBUG: No file info found in recent messages, trying fallback method');
                    }

                    // Fallback: Try the old method as backup
                    return this.extractFileInfoFallback(button);

                } catch (error) {
                    this.log(`Error extracting file info: ${error.message}`);
                    if (this.debugMode) {
                        this.log(`DEBUG: Error stack: ${error.stack}`);
                    }
                    return null;
                }
            }

            // Extract file info from a specific code block
            extractFileInfoFromBlock(block) {
                try {
                    if (this.debugMode) {
                        this.log(`DEBUG: Analyzing block with classes: ${block.className}`);
                    }

                    // Look for filename in multiple possible locations
                    let filename = null;
                    let addedLines = 0;
                    let deletedLines = 0;

                    // Method 1: .composer-code-block-filename span
                    const filenameSpan = block.querySelector('.composer-code-block-filename span[style*="direction: ltr"]') ||
                                        block.querySelector('.composer-code-block-filename span') ||
                                        block.querySelector('.composer-code-block-filename');
                    
                    if (filenameSpan) {
                        filename = filenameSpan.textContent.trim();
                        if (this.debugMode) {
                            this.log(`DEBUG: Found filename via span: "${filename}"`);
                        }
                    }

                    // Method 2: Look for any element with filename-like content
                    if (!filename) {
                        const allSpans = block.querySelectorAll('span');
                        for (const span of allSpans) {
                            const text = span.textContent.trim();
                            // Check if text looks like a filename (has extension)
                            if (text && text.includes('.') && text.length < 100 && !text.includes(' ')) {
                                const parts = text.split('.');
                                if (parts.length >= 2 && parts[parts.length - 1].length <= 10) {
                                    filename = text;
                                    if (this.debugMode) {
                                        this.log(`DEBUG: Found filename via pattern matching: "${filename}"`);
                                    }
                                    break;
                                }
                            }
                        }
                    }

                    // Extract diff stats from status elements
                    const statusElements = block.querySelectorAll('.composer-code-block-status span, span[style*="color"]');
                    
                    if (this.debugMode) {
                        this.log(`DEBUG: Found ${statusElements.length} status elements`);
                    }

                    for (const statusEl of statusElements) {
                        const statusText = statusEl.textContent.trim();
                        if (this.debugMode) {
                            this.log(`DEBUG: Status text: "${statusText}"`);
                        }

                        // Look for +N/-N patterns
                        const addedMatch = statusText.match(/\+(\d+)/);
                        const deletedMatch = statusText.match(/-(\d+)/);
                        
                        if (addedMatch) {
                            addedLines = Math.max(addedLines, parseInt(addedMatch[1]));
                            if (this.debugMode) {
                                this.log(`DEBUG: Found added lines: ${addedLines}`);
                            }
                        }
                        if (deletedMatch) {
                            deletedLines = Math.max(deletedLines, parseInt(deletedMatch[1]));
                            if (this.debugMode) {
                                this.log(`DEBUG: Found deleted lines: ${deletedLines}`);
                            }
                        }
                    }

                    if (filename) {
                        const fileInfo = {
                            filename,
                            addedLines: addedLines || 0,
                            deletedLines: deletedLines || 0,
                            timestamp: new Date()
                        };

                        if (this.debugMode) {
                            this.log(`DEBUG: Created file info object: ${JSON.stringify(fileInfo)}`);
                        }

                        return fileInfo;
                    }

                    if (this.debugMode) {
                        this.log('DEBUG: No filename found in this block');
                    }
                    return null;

                } catch (error) {
                    if (this.debugMode) {
                        this.log(`DEBUG: Error in extractFileInfoFromBlock: ${error.message}`);
                    }
                    return null;
                }
            }

            // Fallback method (original approach)
            extractFileInfoFallback(button) {
                try {
                    if (this.debugMode) {
                        this.log('DEBUG: Using fallback extraction method');
                    }

                    // Look for the composer-code-block-container that contains this button
                    let container = button.closest('.composer-code-block-container');
                    if (!container) {
                        // Try to find it in parent elements
                        let parent = button.parentElement;
                        let attempts = 0;
                        while (parent && attempts < 10) {
                            container = parent.querySelector('.composer-code-block-container');
                            if (container) break;
                            parent = parent.parentElement;
                            attempts++;
                        }
                    }
                    
                    if (!container) {
                        if (this.debugMode) {
                            this.log('DEBUG: No container found in fallback method');
                        }
                        return null;
                    }
                    
                    // Extract filename from .composer-code-block-filename
                    let filenameElement = container.querySelector('.composer-code-block-filename span[style*="direction: ltr"]');
                    if (!filenameElement) {
                        filenameElement = container.querySelector('.composer-code-block-filename span');
                    }
                    if (!filenameElement) {
                        filenameElement = container.querySelector('.composer-code-block-filename');
                    }
                    const filename = filenameElement ? filenameElement.textContent.trim() : 'Unknown';
                    
                    // Extract diff stats from .composer-code-block-status
                    const statusElement = container.querySelector('.composer-code-block-status span');
                    let addedLines = 0;
                    let deletedLines = 0;
                    
                    if (statusElement) {
                        const statusText = statusElement.textContent;
                        const addedMatch = statusText.match(/\+(\d+)/);
                        const deletedMatch = statusText.match(/-(\d+)/);
                        
                        if (addedMatch) addedLines = parseInt(addedMatch[1]);
                        if (deletedMatch) deletedLines = parseInt(deletedMatch[1]);
                        
                        if (this.debugMode) {
                            this.log(`DEBUG: Fallback extracted - File: ${filename}, Status: "${statusText}", +${addedLines}/-${deletedLines}`);
                        }
                    }
                    
                    return {
                        filename,
                        addedLines: addedLines || 0,
                        deletedLines: deletedLines || 0,
                        timestamp: new Date()
                    };
                    
                } catch (error) {
                    if (this.debugMode) {
                        this.log(`DEBUG: Error in fallback method: ${error.message}`);
                    }
                    return null;
                }
            }
            
            // Track file acceptance
            trackFileAcceptance(fileInfo, buttonType = 'accept') {
                if (!fileInfo || !fileInfo.filename) return;
                
                const { filename, addedLines, deletedLines, timestamp } = fileInfo;
                
                // Normalize button type for consistent tracking
                const normalizedButtonType = this.normalizeButtonType(buttonType);
                
                // Calculate time saved for this action  
                const timeSaved = this.calculateTimeSaved(normalizedButtonType);
                
                // Ensure numbers are valid (not NaN)
                const safeAddedLines = isNaN(addedLines) ? 0 : addedLines;
                const safeDeletedLines = isNaN(deletedLines) ? 0 : deletedLines;
                const safeTimeSaved = isNaN(timeSaved) ? 0 : timeSaved;
                
                // Update file statistics
                if (this.analytics.files.has(filename)) {
                    const existing = this.analytics.files.get(filename);
                    existing.acceptCount++;
                    existing.lastAccepted = timestamp;
                    existing.totalAdded += safeAddedLines;
                    existing.totalDeleted += safeDeletedLines;
                    
                    // Track button type counts
                    if (!existing.buttonTypes) {
                        existing.buttonTypes = {};
                    }
                    existing.buttonTypes[normalizedButtonType] = (existing.buttonTypes[normalizedButtonType] || 0) + 1;
                } else {
                    this.analytics.files.set(filename, {
                        acceptCount: 1,
                        firstAccepted: timestamp,
                        lastAccepted: timestamp,
                        totalAdded: safeAddedLines,
                        totalDeleted: safeDeletedLines,
                        buttonTypes: {
                            [normalizedButtonType]: 1
                        }
                    });
                }
                
                // Track in session with separate button type tracking
                this.analytics.sessions.push({
                    filename,
                    addedLines: safeAddedLines,
                    deletedLines: safeDeletedLines,
                    timestamp,
                    buttonType: normalizedButtonType,
                    timeSaved: safeTimeSaved
                });
                
                // Update button type counters
                if (!this.analytics.buttonTypeCounts) {
                    this.analytics.buttonTypeCounts = {};
                }
                this.analytics.buttonTypeCounts[normalizedButtonType] = (this.analytics.buttonTypeCounts[normalizedButtonType] || 0) + 1;
                
                this.analytics.totalAccepts++;
                
                this.logToPanel(`📁 ${filename} (+${safeAddedLines}/-${safeDeletedLines}) [${normalizedButtonType}] [saved ${this.formatTimeDuration(safeTimeSaved)}]`, 'file');
                this.log(`File accepted: ${filename} (+${safeAddedLines}/-${safeDeletedLines}) - Button: ${normalizedButtonType} - Time saved: ${this.formatTimeDuration(safeTimeSaved)}`);
                
                // Update analytics panel if visible
                if (this.currentTab === 'analytics' || this.currentTab === 'roi') {
                    this.updateAnalyticsContent();
                }
                
                // Update main footer ROI display
                this.updateMainFooter();
                
                // Save to storage
                this.saveToStorage();
            }

            // Normalize button type for consistent analytics
            normalizeButtonType(buttonType) {
                if (!buttonType) return 'unknown';
                
                const type = buttonType.toLowerCase().trim();
                
                // Map variations to standard types
                if (type.includes('accept all')) return 'accept-all';
                if (type.includes('accept')) return 'accept';
                if (type.includes('review next file')) return 'review-next-file';
                if (type.includes('keep all')) return 'keep-all';
                if (type.includes('keep')) return 'keep';
                if (type.includes('run command')) return 'run-command';
                if (type.includes('run')) return 'run';
                if (type.includes('apply')) return 'apply';
                if (type.includes('execute')) return 'execute';
                if (type.includes('resume') && type.includes('conversation')) return 'resume-conversation';
                if (type.includes('resume')) return 'connection-resume'; // Connection failure resume
                if (type.includes('try again')) return 'try-again';
                
                return type;
            }
            
            createControlPanel() {
                if (this.controlPanel) return;
                
                this.controlPanel = document.createElement('div');
                this.controlPanel.id = 'auto-accept-control-panel';
                
                // Create header with tabs
                const header = document.createElement('div');
                header.className = 'aa-header';
                
                const tabsContainer = document.createElement('div');
                tabsContainer.className = 'aa-tabs';
                
                const mainTab = document.createElement('button');
                mainTab.className = 'aa-tab aa-tab-active';
                mainTab.textContent = 'Main';
                mainTab.onclick = () => this.switchTab('main');
                
                const analyticsTab = document.createElement('button');
                analyticsTab.className = 'aa-tab';
                analyticsTab.textContent = 'Analytics';
                analyticsTab.onclick = () => this.switchTab('analytics');
                
                const roiTab = document.createElement('button');
                roiTab.className = 'aa-tab';
                roiTab.textContent = 'ROI';
                roiTab.onclick = () => this.switchTab('roi');
                
                tabsContainer.appendChild(mainTab);
                tabsContainer.appendChild(analyticsTab);
                tabsContainer.appendChild(roiTab);
                
                const headerControls = document.createElement('div');
                headerControls.className = 'aa-header-controls';
                
                const minimizeBtn = document.createElement('button');
                minimizeBtn.className = 'aa-minimize';
                minimizeBtn.title = 'Minimize';
                minimizeBtn.textContent = '−';
                
                const closeBtn = document.createElement('button');
                closeBtn.className = 'aa-close';
                closeBtn.title = 'Close';
                closeBtn.textContent = '×';
                
                headerControls.appendChild(minimizeBtn);
                headerControls.appendChild(closeBtn);
                
                header.appendChild(tabsContainer);
                header.appendChild(headerControls);
                
                // Create main content area
                const mainContent = document.createElement('div');
                mainContent.className = 'aa-content aa-main-content';
                
                // Status section
                const status = document.createElement('div');
                status.className = 'aa-status';
                
                const statusText = document.createElement('span');
                statusText.className = 'aa-status-text';
                statusText.textContent = 'Stopped';
                
                const clicksText = document.createElement('span');
                clicksText.className = 'aa-clicks';
                clicksText.textContent = '0 clicks';
                
                status.appendChild(statusText);
                status.appendChild(clicksText);
                
                // Controls section
                const controls = document.createElement('div');
                controls.className = 'aa-controls';
                
                const startBtn = document.createElement('button');
                startBtn.className = 'aa-btn aa-start';
                startBtn.textContent = 'Start';
                
                const stopBtn = document.createElement('button');
                stopBtn.className = 'aa-btn aa-stop';
                stopBtn.textContent = 'Stop';
                stopBtn.disabled = true;
                
                const configBtn = document.createElement('button');
                configBtn.className = 'aa-btn aa-config';
                configBtn.textContent = 'Config';
                
                controls.appendChild(startBtn);
                controls.appendChild(stopBtn);
                controls.appendChild(configBtn);
                
                // Config panel
                const configPanel = document.createElement('div');
                configPanel.className = 'aa-config-panel';
                configPanel.style.display = 'none';
                
                const configOptions = [
                    { id: 'aa-accept-all', text: 'Accept All', checked: true },
                    { id: 'aa-accept', text: 'Accept', checked: true },
                    { id: 'aa-review-next-file', text: 'Review Next File', checked: true },
                    { id: 'aa-keep-all', text: 'Keep All', checked: true },
                    { id: 'aa-keep', text: 'Keep', checked: true },
                    { id: 'aa-run', text: 'Run', checked: true },
                    { id: 'aa-apply', text: 'Apply', checked: true },
                    { id: 'aa-resume', text: 'Resume Conversation', checked: true },
                    { id: 'aa-connection-resume', text: 'Connection Resume', checked: true },
                    { id: 'aa-try-again', text: 'Try Again', checked: true }
                ];
                
                configOptions.forEach(option => {
                    const label = document.createElement('label');
                    const checkbox = document.createElement('input');
                    checkbox.type = 'checkbox';
                    checkbox.id = option.id;
                    checkbox.checked = option.checked;
                    
                    label.appendChild(checkbox);
                    label.appendChild(document.createTextNode(' ' + option.text));
                    configPanel.appendChild(label);
                });
                
                // Log section
                const log = document.createElement('div');
                log.className = 'aa-log';
                
                // ROI Footer for main tab
                const roiFooter = document.createElement('div');
                roiFooter.className = 'aa-roi-footer';
                
                // Credits section for main tab
                const creditsDiv = document.createElement('div');
                creditsDiv.className = 'aa-credits';
                
                const creditsText = document.createElement('small');
                creditsText.textContent = 'Created by ';
                
                const creditsLink = document.createElement('a');
                creditsLink.href = 'https://linkedin.com/in/ivalsaraj';
                creditsLink.target = '_blank';
                creditsLink.textContent = '@ivalsaraj';
                
                creditsText.appendChild(creditsLink);
                creditsDiv.appendChild(creditsText);
                
                // Assemble main content
                mainContent.appendChild(status);
                mainContent.appendChild(controls);
                mainContent.appendChild(configPanel);
                mainContent.appendChild(log);
                mainContent.appendChild(roiFooter);
                mainContent.appendChild(creditsDiv);
                
                // Create analytics content area
                const analyticsContent = document.createElement('div');
                analyticsContent.className = 'aa-content aa-analytics-content';
                analyticsContent.style.display = 'none';
                
                // Assemble everything
                this.controlPanel.appendChild(header);
                this.controlPanel.appendChild(mainContent);
                this.controlPanel.appendChild(analyticsContent);
                
                this.controlPanel.style.cssText = `
                    position: fixed !important;
                    top: 100px !important;
                    right: 20px !important;
                    width: 320px !important;
                    background: #1e1e1e !important;
                    border: 2px solid #4CAF50 !important;
                    border-radius: 8px !important;
                    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.8) !important;
                    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif !important;
                    font-size: 12px !important;
                    color: #ccc !important;
                    z-index: 999999 !important;
                    user-select: none !important;
                    max-height: 600px !important;
                    display: flex !important;
                    flex-direction: column !important;
                    opacity: 1 !important;
                    visibility: visible !important;
                `;
                
                this.addPanelStyles();
                this.setupPanelEvents();
                document.body.appendChild(this.controlPanel);
                
                // Initialize analytics content and update main footer
                this.updateAnalyticsContent();
                this.updateMainFooter();
            }
            
            updateMainFooter() {
                const roiFooter = this.controlPanel?.querySelector('.aa-roi-footer');
                if (!roiFooter) return;
                
                // Clear existing content
                while (roiFooter.firstChild) {
                    roiFooter.removeChild(roiFooter.firstChild);
                }
                
                // Calculate ROI metrics
                const totalTimeSaved = this.roiTracking.totalTimeSaved || 0;
                const totalAccepts = this.analytics.totalAccepts || 0;
                const sessionDuration = new Date() - this.analytics.sessionStart;
                
                // Safe calculations to avoid NaN - Calculate efficiency based on complete workflow
                const averageManualWorkflowTime = this.roiTracking.averageCompleteWorkflow;
                const totalManualTime = totalAccepts * averageManualWorkflowTime;
                const totalAutomatedTime = totalAccepts * this.roiTracking.averageAutomatedWorkflow;
                const efficiencyGain = totalManualTime > 0 ? 
                    ((totalManualTime - totalAutomatedTime) / totalManualTime * 100) : 0;
                
                const title = document.createElement('div');
                title.className = 'aa-roi-footer-title';
                title.textContent = '⚡ Workflow ROI';
                
                const stats = document.createElement('div');
                stats.className = 'aa-roi-footer-stats';
                
                const timeSavedSpan = document.createElement('span');
                timeSavedSpan.textContent = `Time Saved: ${this.formatTimeDuration(totalTimeSaved)}`;
                
                const efficiencySpan = document.createElement('span');
                efficiencySpan.textContent = `Workflow Efficiency: ${efficiencyGain.toFixed(1)}%`;
                
                stats.appendChild(timeSavedSpan);
                stats.appendChild(efficiencySpan);
                
                roiFooter.appendChild(title);
                roiFooter.appendChild(stats);
            }
            
            switchTab(tabName) {
                this.currentTab = tabName;
                
                // Update tab buttons
                const tabs = this.controlPanel.querySelectorAll('.aa-tab');
                tabs.forEach(tab => {
                    tab.classList.remove('aa-tab-active');
                    if (tab.textContent.toLowerCase() === tabName) {
                        tab.classList.add('aa-tab-active');
                    }
                });
                
                // Update content visibility
                const mainContent = this.controlPanel.querySelector('.aa-main-content');
                const analyticsContent = this.controlPanel.querySelector('.aa-analytics-content');
                
                if (tabName === 'main') {
                    mainContent.style.display = 'block';
                    analyticsContent.style.display = 'none';
                } else if (tabName === 'analytics') {
                    mainContent.style.display = 'none';
                    analyticsContent.style.display = 'block';
                    this.updateAnalyticsContent();
                } else if (tabName === 'roi') {
                    mainContent.style.display = 'none';
                    analyticsContent.style.display = 'block';
                    this.updateAnalyticsContent();
                }
            }
            
            updateAnalyticsContent() {
                const analyticsContent = this.controlPanel.querySelector('.aa-analytics-content');
                if (!analyticsContent) return;
                
                // Clear existing content
                analyticsContent.textContent = '';
                
                if (this.currentTab === 'analytics') {
                    this.renderAnalyticsTab(analyticsContent);
                } else if (this.currentTab === 'roi') {
                    this.renderROITab(analyticsContent);
                }
            }
            
            renderAnalyticsTab(container) {
                const now = new Date();
                const sessionDuration = Math.round((now - this.analytics.sessionStart) / 1000 / 60); // minutes
                
                // Calculate totals
                let totalFiles = this.analytics.files.size;
                let totalAdded = 0;
                let totalDeleted = 0;
                
                this.analytics.files.forEach(fileData => {
                    totalAdded += fileData.totalAdded;
                    totalDeleted += fileData.totalDeleted;
                });
                
                // Create analytics summary
                const summaryDiv = document.createElement('div');
                summaryDiv.className = 'aa-analytics-summary';
                
                const summaryTitle = document.createElement('h4');
                summaryTitle.textContent = '📊 Session Analytics';
                summaryDiv.appendChild(summaryTitle);
                
                const stats = [
                    { label: 'Session:', value: `${sessionDuration}min` },
                    { label: 'Total Accepts:', value: `${this.analytics.totalAccepts}` },
                    { label: 'Files Modified:', value: `${totalFiles}` },
                    { label: 'Lines Added:', value: `+${totalAdded}`, class: 'aa-stat-added' },
                    { label: 'Lines Deleted:', value: `-${totalDeleted}`, class: 'aa-stat-deleted' }
                ];
                
                stats.forEach(stat => {
                    const statDiv = document.createElement('div');
                    statDiv.className = 'aa-stat';
                    
                    const labelSpan = document.createElement('span');
                    labelSpan.className = 'aa-stat-label';
                    labelSpan.textContent = stat.label;
                    
                    const valueSpan = document.createElement('span');
                    valueSpan.className = `aa-stat-value ${stat.class || ''}`;
                    valueSpan.textContent = stat.value;
                    
                    statDiv.appendChild(labelSpan);
                    statDiv.appendChild(valueSpan);
                    summaryDiv.appendChild(statDiv);
                });

                // Add button type breakdown
                if (this.analytics.buttonTypeCounts && Object.keys(this.analytics.buttonTypeCounts).length > 0) {
                    const buttonTypeDiv = document.createElement('div');
                    buttonTypeDiv.className = 'aa-button-types';
                    
                    const buttonTypeTitle = document.createElement('h5');
                    buttonTypeTitle.textContent = '🎯 Button Types';
                    buttonTypeTitle.style.cssText = 'margin: 8px 0 4px 0; font-size: 11px; color: #ddd;';
                    buttonTypeDiv.appendChild(buttonTypeTitle);
                    
                    Object.entries(this.analytics.buttonTypeCounts).forEach(([type, count]) => {
                        const typeDiv = document.createElement('div');
                        typeDiv.className = 'aa-stat aa-button-type-stat';
                        typeDiv.style.cssText = 'font-size: 10px; padding: 2px 0;';
                        
                        const labelSpan = document.createElement('span');
                        labelSpan.className = 'aa-stat-label';
                        labelSpan.textContent = `${type}:`;
                        
                        const valueSpan = document.createElement('span');
                        valueSpan.className = 'aa-stat-value';
                        valueSpan.textContent = `${count}x`;
                        
                        // Add type-specific styling
                        if (type === 'accept' || type === 'accept-all') {
                            valueSpan.style.color = '#4CAF50';
                        } else if (type === 'run' || type === 'run-command') {
                            valueSpan.style.color = '#FF9800';
                        } else if (type === 'resume-conversation') {
                            valueSpan.style.color = '#2196F3';
                        } else if (type === 'connection-resume' || type === 'try-again') {
                            valueSpan.style.color = '#FF5722'; // Orange-red for connection issues
                        } else {
                            valueSpan.style.color = '#9C27B0';
                        }
                        
                        typeDiv.appendChild(labelSpan);
                        typeDiv.appendChild(valueSpan);
                        buttonTypeDiv.appendChild(typeDiv);
                    });
                    
                    summaryDiv.appendChild(buttonTypeDiv);
                }
                
                // Create files section
                const filesDiv = document.createElement('div');
                filesDiv.className = 'aa-analytics-files';
                
                const filesTitle = document.createElement('h4');
                filesTitle.textContent = '📁 File Activity';
                filesDiv.appendChild(filesTitle);
                
                const filesList = document.createElement('div');
                filesList.className = 'aa-files-list';
                this.renderFilesList(filesList);
                filesDiv.appendChild(filesList);
                
                // Create actions section
                const actionsDiv = document.createElement('div');
                actionsDiv.className = 'aa-analytics-actions';
                
                const exportBtn = document.createElement('button');
                exportBtn.className = 'aa-btn aa-btn-small';
                exportBtn.textContent = 'Export Data';
                exportBtn.onclick = () => this.exportAnalytics();
                
                const clearBtn = document.createElement('button');
                clearBtn.className = 'aa-btn aa-btn-small';
                clearBtn.textContent = 'Clear Data';
                clearBtn.onclick = () => this.clearAnalytics();
                
                actionsDiv.appendChild(exportBtn);
                actionsDiv.appendChild(clearBtn);
                
                // Create credits section
                const creditsDiv = document.createElement('div');
                creditsDiv.className = 'aa-credits';
                
                const creditsText = document.createElement('small');
                creditsText.textContent = 'Created by ';
                
                const creditsLink = document.createElement('a');
                creditsLink.href = 'https://linkedin.com/in/ivalsaraj';
                creditsLink.target = '_blank';
                creditsLink.textContent = '@ivalsaraj';
                
                creditsText.appendChild(creditsLink);
                creditsDiv.appendChild(creditsText);
                
                // Append all sections
                container.appendChild(summaryDiv);
                container.appendChild(filesDiv);
                container.appendChild(actionsDiv);
                container.appendChild(creditsDiv);
            }
            
            renderROITab(container) {
                const now = new Date();
                const sessionDuration = now - this.analytics.sessionStart;
                
                // Calculate ROI metrics with safe fallbacks
                const totalTimeSaved = this.roiTracking.totalTimeSaved || 0;
                const totalAccepts = this.analytics.totalAccepts || 0;
                const averageTimePerClick = totalAccepts > 0 ? totalTimeSaved / totalAccepts : 0;
                const productivityGain = sessionDuration > 0 ? (totalTimeSaved / sessionDuration) * 100 : 0;
                
                // Create ROI summary
                const summaryDiv = document.createElement('div');
                summaryDiv.className = 'aa-roi-summary';
                
                const summaryTitle = document.createElement('h4');
                summaryTitle.textContent = '⚡ Complete Workflow ROI';
                summaryDiv.appendChild(summaryTitle);
                
                // Add explanation of workflow measurement
                const explanationDiv = document.createElement('div');
                explanationDiv.className = 'aa-roi-explanation';
                explanationDiv.style.cssText = 'font-size: 10px; color: #888; margin-bottom: 8px; line-height: 1.3;';
                explanationDiv.textContent = 'Measures complete AI workflow: User prompt → Cursor generation → Manual watching/clicking vs Auto-acceptance';
                summaryDiv.appendChild(explanationDiv);
                
                const roiStats = [
                    { label: 'Total Time Saved:', value: this.formatTimeDuration(totalTimeSaved), class: 'aa-roi-highlight' },
                    { label: 'Session Duration:', value: this.formatTimeDuration(sessionDuration) },
                    { label: 'Avg. per Click:', value: this.formatTimeDuration(averageTimePerClick) },
                    { label: 'Productivity Gain:', value: `${productivityGain.toFixed(1)}%`, class: 'aa-roi-percentage' },
                    { label: 'Clicks Automated:', value: `${totalAccepts}` }
                ];
                
                roiStats.forEach(stat => {
                    const statDiv = document.createElement('div');
                    statDiv.className = 'aa-stat';
                    
                    const labelSpan = document.createElement('span');
                    labelSpan.className = 'aa-stat-label';
                    labelSpan.textContent = stat.label;
                    
                    const valueSpan = document.createElement('span');
                    valueSpan.className = `aa-stat-value ${stat.class || ''}`;
                    valueSpan.textContent = stat.value;
                    
                    statDiv.appendChild(labelSpan);
                    statDiv.appendChild(valueSpan);
                    summaryDiv.appendChild(statDiv);
                });
                
                // Create impact analysis
                const impactDiv = document.createElement('div');
                impactDiv.className = 'aa-roi-impact';
                
                const impactTitle = document.createElement('h4');
                impactTitle.textContent = '📈 Impact Analysis';
                impactDiv.appendChild(impactTitle);
                
                const impactText = document.createElement('div');
                impactText.className = 'aa-roi-text';
                
                // Calculate different scenarios with safe division
                const hourlyRate = sessionDuration > 0 ? (totalTimeSaved / sessionDuration) : 0;
                const dailyProjection = hourlyRate * (8 * 60 * 60 * 1000); // 8 hour workday
                const weeklyProjection = dailyProjection * 5;
                const monthlyProjection = dailyProjection * 22; // work days
                
                const scenarios = [
                    { period: 'Daily (8hrs)', saved: dailyProjection },
                    { period: 'Weekly (5 days)', saved: weeklyProjection },
                    { period: 'Monthly (22 days)', saved: monthlyProjection }
                ];
                
                scenarios.forEach(scenario => {
                    const scenarioDiv = document.createElement('div');
                    scenarioDiv.className = 'aa-roi-scenario';
                    scenarioDiv.textContent = `${scenario.period}: ${this.formatTimeDuration(scenario.saved)} saved`;
                    impactText.appendChild(scenarioDiv);
                });
                
                impactDiv.appendChild(impactText);
                
                // Manual vs Automated comparison
                const comparisonDiv = document.createElement('div');
                comparisonDiv.className = 'aa-roi-comparison';
                
                const comparisonTitle = document.createElement('h4');
                comparisonTitle.textContent = '🔄 Complete Workflow Comparison';
                comparisonDiv.appendChild(comparisonTitle);
                
                // Add workflow breakdown explanation  
                const workflowBreakdown = document.createElement('div');
                workflowBreakdown.className = 'aa-workflow-breakdown';
                workflowBreakdown.style.cssText = 'font-size: 10px; color: #888; margin-bottom: 8px; line-height: 1.3;';
                
                const manualLine = document.createElement('div');
                manualLine.textContent = 'Manual: Watch generation + Find button + Click + Context switch (~30s)';
                workflowBreakdown.appendChild(manualLine);
                
                const automatedLine = document.createElement('div');
                automatedLine.textContent = 'Automated: Instant detection and clicking while you code (~0.1s)';
                workflowBreakdown.appendChild(automatedLine);
                
                comparisonDiv.appendChild(workflowBreakdown);
                
                const manualTime = totalAccepts * this.roiTracking.averageCompleteWorkflow;
                const automatedTime = totalAccepts * this.roiTracking.averageAutomatedWorkflow;
                
                const comparisonStats = [
                    { label: 'Manual Workflow Time:', value: this.formatTimeDuration(manualTime), class: 'aa-roi-manual' },
                    { label: 'Automated Workflow Time:', value: this.formatTimeDuration(automatedTime), class: 'aa-roi-auto' },
                    { label: 'Workflow Efficiency:', value: `${manualTime > 0 ? ((manualTime - automatedTime) / manualTime * 100).toFixed(1) : '0.0'}%`, class: 'aa-roi-highlight' }
                ];
                
                comparisonStats.forEach(stat => {
                    const statDiv = document.createElement('div');
                    statDiv.className = 'aa-stat';
                    
                    const labelSpan = document.createElement('span');
                    labelSpan.className = 'aa-stat-label';
                    labelSpan.textContent = stat.label;
                    
                    const valueSpan = document.createElement('span');
                    valueSpan.className = `aa-stat-value ${stat.class || ''}`;
                    valueSpan.textContent = stat.value;
                    
                    statDiv.appendChild(labelSpan);
                    statDiv.appendChild(valueSpan);
                    comparisonDiv.appendChild(statDiv);
                });
                
                // Create credits section for ROI tab too
                const creditsDiv = document.createElement('div');
                creditsDiv.className = 'aa-credits';
                
                const creditsText = document.createElement('small');
                creditsText.textContent = 'Created by ';
                
                const creditsLink = document.createElement('a');
                creditsLink.href = 'https://linkedin.com/in/ivalsaraj';
                creditsLink.target = '_blank';
                creditsLink.textContent = '@ivalsaraj';
                
                creditsText.appendChild(creditsLink);
                creditsDiv.appendChild(creditsText);
                
                // Append all sections
                container.appendChild(summaryDiv);
                container.appendChild(impactDiv);
                container.appendChild(comparisonDiv);
                container.appendChild(creditsDiv);
            }
            
            renderFilesList(container) {
                if (this.analytics.files.size === 0) {
                    const noFilesDiv = document.createElement('div');
                    noFilesDiv.className = 'aa-no-files';
                    noFilesDiv.textContent = 'No files modified yet';
                    container.appendChild(noFilesDiv);
                    return;
                }
                
                const sortedFiles = Array.from(this.analytics.files.entries())
                    .sort((a, b) => b[1].lastAccepted - a[1].lastAccepted);
                
                sortedFiles.forEach(([filename, data]) => {
                    const timeAgo = this.getTimeAgo(data.lastAccepted);
                    
                    const fileItem = document.createElement('div');
                    fileItem.className = 'aa-file-item';
                    
                    const fileName = document.createElement('div');
                    fileName.className = 'aa-file-name';
                    fileName.textContent = filename;
                    
                    const fileStats = document.createElement('div');
                    fileStats.className = 'aa-file-stats';
                    
                    const fileCount = document.createElement('span');
                    fileCount.className = 'aa-file-count';
                    fileCount.textContent = `${data.acceptCount}x`;
                    
                    const fileChanges = document.createElement('span');
                    fileChanges.className = 'aa-file-changes';
                    fileChanges.textContent = `+${data.totalAdded}/-${data.totalDeleted}`;
                    
                    const fileTime = document.createElement('span');
                    fileTime.className = 'aa-file-time';
                    fileTime.textContent = timeAgo;
                    
                    fileStats.appendChild(fileCount);
                    fileStats.appendChild(fileChanges);
                    fileStats.appendChild(fileTime);
                    
                    fileItem.appendChild(fileName);
                    fileItem.appendChild(fileStats);
                    
                    container.appendChild(fileItem);
                });
            }
            
            getTimeAgo(date) {
                const now = new Date();
                const diff = Math.round((now - date) / 1000); // seconds
                
                if (diff < 60) return `${diff}s ago`;
                if (diff < 3600) return `${Math.round(diff/60)}m ago`;
                if (diff < 86400) return `${Math.round(diff/3600)}h ago`;
                return `${Math.round(diff/86400)}d ago`;
            }
            
            exportAnalytics() {
                const data = {
                    session: {
                        start: this.analytics.sessionStart,
                        duration: new Date() - this.analytics.sessionStart,
                        totalAccepts: this.analytics.totalAccepts
                    },
                    files: Object.fromEntries(this.analytics.files),
                    sessions: this.analytics.sessions,
                    config: this.config,
                    exportedAt: new Date()
                };
                
                const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `cursor-auto-accept-analytics-${new Date().toISOString().split('T')[0]}.json`;
                a.click();
                URL.revokeObjectURL(url);
                
                this.logToPanel('📥 Analytics exported', 'info');
            }
            
            clearAnalytics() {
                if (confirm('Clear all analytics data? This cannot be undone.')) {
                    this.analytics.files.clear();
                    this.analytics.sessions = [];
                    this.analytics.totalAccepts = 0;
                    this.analytics.sessionStart = new Date();
                    this.updateAnalyticsContent();
                    this.logToPanel('🗑️ Analytics cleared', 'warning');
                }
            }
            
            addPanelStyles() {
                if (document.getElementById('auto-accept-styles')) return;
                
                const style = document.createElement('style');
                style.id = 'auto-accept-styles';
                style.textContent = `
                    .aa-header {
                        background: #2d2d2d;
                        padding: 6px 10px;
                        border-radius: 5px 5px 0 0;
                        cursor: move;
                        display: flex;
                        justify-content: space-between;
                        align-items: center;
                        border-bottom: 1px solid #333;
                    }
                    
                    .aa-tabs {
                        display: flex;
                        gap: 4px;
                    }
                    
                    .aa-tab {
                        background: #444;
                        border: none;
                        color: #ccc;
                        font-size: 11px;
                        cursor: pointer;
                        padding: 4px 8px;
                        border-radius: 3px;
                        transition: all 0.2s;
                    }
                    
                    .aa-tab:hover {
                        background: #555;
                    }
                    
                    .aa-tab-active {
                        background: #0d7377 !important;
                        color: white !important;
                    }
                    
                    .aa-header-controls {
                        display: flex;
                        gap: 4px;
                    }
                    
                    .aa-title {
                        font-weight: 500;
                        color: #fff;
                        font-size: 12px;
                    }
                    .aa-minimize, .aa-close {
                        background: #444;
                        border: none;
                        color: #ccc;
                        font-size: 12px;
                        font-weight: bold;
                        cursor: pointer;
                        padding: 2px 5px;
                        border-radius: 2px;
                        line-height: 1;
                        width: 16px;
                        height: 16px;
                        display: flex;
                        align-items: center;
                        justify-content: center;
                    }
                    .aa-minimize:hover, .aa-close:hover {
                        background: #555;
                    }
                    .aa-close:hover {
                        background: #dc3545;
                        color: white;
                    }
                    .aa-content {
                        padding: 12px;
                        overflow-y: auto;
                        flex: 1;
                    }
                    .aa-status {
                        display: flex;
                        justify-content: space-between;
                        margin-bottom: 10px;
                        padding: 6px 8px;
                        background: #252525;
                        border-radius: 4px;
                        font-size: 11px;
                    }
                    .aa-status-text.running {
                        color: #4CAF50;
                        font-weight: 500;
                    }
                    .aa-status-text.stopped {
                        color: #f44336;
                    }
                    .aa-clicks {
                        color: #888;
                    }
                    .aa-controls {
                        display: flex;
                        gap: 6px;
                        margin-bottom: 10px;
                    }
                    .aa-btn {
                        flex: 1;
                        padding: 6px 12px;
                        border: none;
                        border-radius: 4px;
                        cursor: pointer;
                        font-size: 11px;
                        font-weight: 500;
                        transition: all 0.2s;
                    }
                    .aa-btn-small {
                        flex: none;
                        padding: 4px 8px;
                        font-size: 10px;
                    }
                    .aa-start {
                        background: #4CAF50;
                        color: white;
                    }
                    .aa-start:hover:not(:disabled) {
                        background: #45a049;
                    }
                    .aa-stop {
                        background: #f44336;
                        color: white;
                    }
                    .aa-stop:hover:not(:disabled) {
                        background: #da190b;
                    }
                    .aa-config {
                        background: #2196F3;
                        color: white;
                    }
                    .aa-config:hover:not(:disabled) {
                        background: #1976D2;
                    }
                    .aa-btn:disabled {
                        opacity: 0.5;
                        cursor: not-allowed;
                    }
                    .aa-config-panel {
                        background: #252525;
                        border-radius: 4px;
                        padding: 8px;
                        margin-bottom: 10px;
                    }
                    .aa-config-panel label {
                        display: block;
                        margin-bottom: 4px;
                        font-size: 11px;
                        cursor: pointer;
                    }
                    .aa-config-panel input[type="checkbox"] {
                        margin-right: 6px;
                    }
                    .aa-log {
                        background: #252525;
                        border-radius: 4px;
                        padding: 8px;
                        height: 120px;
                        overflow-y: auto;
                        font-size: 10px;
                        line-height: 1.3;
                    }
                    .aa-log-entry {
                        margin-bottom: 2px;
                        padding: 2px 4px;
                        border-radius: 2px;
                    }
                    .aa-log-entry.info {
                        color: #4CAF50;
                    }
                    .aa-log-entry.warning {
                        color: #FF9800;
                    }
                    .aa-log-entry.error {
                        color: #f44336;
                    }
                    .aa-log-entry.file {
                        color: #2196F3;
                        background: rgba(33, 150, 243, 0.1);
                    }
                    
                    /* Analytics Styles */
                    .aa-analytics-summary {
                        background: #252525;
                        border-radius: 4px;
                        padding: 8px;
                        margin-bottom: 10px;
                    }
                    
                    .aa-analytics-summary h4 {
                        margin: 0 0 8px 0;
                        font-size: 12px;
                        color: #fff;
                    }
                    
                    .aa-stat {
                        display: flex;
                        justify-content: space-between;
                        margin-bottom: 4px;
                        font-size: 11px;
                    }
                    
                    .aa-stat-label {
                        color: #888;
                    }
                    
                    .aa-stat-value {
                        color: #fff;
                        font-weight: 500;
                    }
                    
                    .aa-stat-added {
                        color: #4CAF50;
                    }
                    
                    .aa-stat-deleted {
                        color: #f44336;
                    }
                    
                    .aa-analytics-files {
                        background: #252525;
                        border-radius: 4px;
                        padding: 8px;
                        margin-bottom: 10px;
                    }
                    
                    .aa-analytics-files h4 {
                        margin: 0 0 8px 0;
                        font-size: 12px;
                        color: #fff;
                    }
                    
                    .aa-files-list {
                        max-height: 150px;
                        overflow-y: auto;
                    }
                    
                    .aa-file-item {
                        padding: 4px 0;
                        border-bottom: 1px solid #333;
                    }
                    
                    .aa-file-item:last-child {
                        border-bottom: none;
                    }
                    
                    .aa-file-name {
                        font-size: 11px;
                        color: #fff;
                        font-weight: 500;
                        margin-bottom: 2px;
                        word-break: break-all;
                    }
                    
                    .aa-file-stats {
                        display: flex;
                        gap: 8px;
                        font-size: 10px;
                        color: #888;
                    }
                    
                    .aa-file-count {
                        background: #444;
                        padding: 1px 4px;
                        border-radius: 2px;
                        color: #ccc;
                    }
                    
                    .aa-file-changes {
                        color: #2196F3;
                    }
                    
                    .aa-file-time {
                        margin-left: auto;
                    }
                    
                    .aa-no-files {
                        color: #888;
                        font-size: 11px;
                        text-align: center;
                        padding: 20px;
                    }
                    
                    .aa-analytics-actions {
                        display: flex;
                        gap: 6px;
                        margin-bottom: 10px;
                    }
                    
                    .aa-analytics-actions .aa-btn {
                        background: #444;
                        color: #ccc;
                    }
                    
                    .aa-analytics-actions .aa-btn:hover {
                        background: #555;
                    }
                    
                    .aa-credits {
                        text-align: center;
                        padding: 8px;
                        border-top: 1px solid #333;
                        color: #666;
                    }
                    
                    .aa-credits a {
                        color: #2196F3;
                        text-decoration: none;
                    }
                    
                    .aa-credits a:hover {
                        text-decoration: underline;
                    }
                    
                    /* ROI Tab Styles */
                    .aa-roi-summary, .aa-roi-impact, .aa-roi-comparison {
                        margin-bottom: 12px;
                        padding: 8px;
                        background: #333;
                        border-radius: 4px;
                    }
                    
                    .aa-roi-highlight {
                        color: #4CAF50 !important;
                        font-weight: 600;
                    }
                    
                    .aa-roi-percentage {
                        color: #2196F3 !important;
                        font-weight: 600;
                    }
                    
                    .aa-roi-manual {
                        color: #FF9800 !important;
                    }
                    
                    .aa-roi-auto {
                        color: #4CAF50 !important;
                    }
                    
                    .aa-roi-text {
                        margin-top: 8px;
                    }
                    
                    .aa-roi-scenario {
                        margin: 4px 0;
                        padding: 4px;
                        background: #444;
                        border-radius: 3px;
                        font-size: 11px;
                        color: #ccc;
                    }
                    
                    /* ROI Footer Styles (for main tab) */
                    .aa-roi-footer {
                        margin-top: 8px;
                        padding: 6px 8px;
                        background: #2d2d2d;
                        border-radius: 4px;
                        border-top: 1px solid #444;
                    }
                    
                    .aa-roi-footer-title {
                        font-size: 10px;
                        color: #fff;
                        font-weight: 600;
                        margin-bottom: 4px;
                    }
                    
                    .aa-roi-footer-stats {
                        display: flex;
                        justify-content: space-between;
                        font-size: 9px;
                        color: #888;
                    }
                    
                    .aa-roi-footer-stats span {
                        color: #4CAF50;
                    }
                    
                    /* Minimize functionality */
                    #auto-accept-control-panel.aa-minimized .aa-content {
                        display: none;
                    }
                    
                    #auto-accept-control-panel.aa-minimized {
                        height: auto;
                        max-height: none;
                    }
                `;
                document.head.appendChild(style);
            }
            
            setupPanelEvents() {
                const header = this.controlPanel.querySelector('.aa-header');
                const minimizeBtn = this.controlPanel.querySelector('.aa-minimize');
                const closeBtn = this.controlPanel.querySelector('.aa-close');
                const startBtn = this.controlPanel.querySelector('.aa-start');
                const stopBtn = this.controlPanel.querySelector('.aa-stop');
                const configBtn = this.controlPanel.querySelector('.aa-config');
                const configPanel = this.controlPanel.querySelector('.aa-config-panel');
                
                // Dragging functionality
                header.addEventListener('mousedown', (e) => {
                    if (e.target === minimizeBtn || e.target === closeBtn) return;
                    this.isDragging = true;
                    const rect = this.controlPanel.getBoundingClientRect();
                    this.dragOffset.x = e.clientX - rect.left;
                    this.dragOffset.y = e.clientY - rect.top;
                    e.preventDefault();
                });
                
                document.addEventListener('mousemove', (e) => {
                    if (!this.isDragging) return;
                    const x = e.clientX - this.dragOffset.x;
                    const y = e.clientY - this.dragOffset.y;
                    this.controlPanel.style.left = Math.max(0, Math.min(window.innerWidth - this.controlPanel.offsetWidth, x)) + 'px';
                    this.controlPanel.style.top = Math.max(0, Math.min(window.innerHeight - this.controlPanel.offsetHeight, y)) + 'px';
                    this.controlPanel.style.right = 'auto';
                });
                
                document.addEventListener('mouseup', () => {
                    this.isDragging = false;
                });
                
                // Control buttons
                minimizeBtn.addEventListener('click', () => {
                    this.controlPanel.classList.toggle('aa-minimized');
                });
                
                closeBtn.addEventListener('click', () => {
                    this.hideControlPanel();
                });
                
                startBtn.addEventListener('click', () => {
                    this.start();
                });
                
                stopBtn.addEventListener('click', () => {
                    this.stop();
                });
                
                configBtn.addEventListener('click', () => {
                    configPanel.style.display = configPanel.style.display === 'none' ? 'block' : 'none';
                });
                
                // Config checkboxes
                const checkboxes = this.controlPanel.querySelectorAll('.aa-config-panel input[type="checkbox"]');
                checkboxes.forEach(checkbox => {
                    checkbox.addEventListener('change', () => {
                        const configMap = {
                            'aa-accept-all': 'enableAcceptAll',
                            'aa-accept': 'enableAccept',
                            'aa-review-next-file': 'enableReviewNextFile',
                            'aa-keep-all': 'enableKeepAll',
                            'aa-keep': 'enableKeep',
                            'aa-run': 'enableRun',
                            'aa-apply': 'enableApply',
                            'aa-resume': 'enableResume',
                            'aa-connection-resume': 'enableConnectionResume',
                            'aa-try-again': 'enableTryAgain'
                        };
                        const configKey = configMap[checkbox.id];
                        if (configKey) {
                            this.config[configKey] = checkbox.checked;
                            this.config.enableRunCommand = this.config.enableRun;
                            this.config.enableExecute = this.config.enableApply;
                            
                            // Save configuration changes
                            this.saveToStorage();
                        }
                    });
                });
            }
            
            updatePanelStatus() {
                if (!this.controlPanel) return;
                
                const statusText = this.controlPanel.querySelector('.aa-status-text');
                const clicksText = this.controlPanel.querySelector('.aa-clicks');
                const startBtn = this.controlPanel.querySelector('.aa-start');
                const stopBtn = this.controlPanel.querySelector('.aa-stop');
                
                if (this.isRunning) {
                    statusText.textContent = 'Running';
                    statusText.className = 'aa-status-text running';
                    startBtn.disabled = true;
                    stopBtn.disabled = false;
                } else {
                    statusText.textContent = 'Stopped';
                    statusText.className = 'aa-status-text stopped';
                    startBtn.disabled = false;
                    stopBtn.disabled = true;
                }
                
                clicksText.textContent = `${this.totalClicks} clicks`;
            }
            
            logToPanel(message, type = 'info') {
                if (!this.controlPanel) return;
                
                // Create unique message key to prevent duplicates
                const messageKey = `${type}:${message}`;
                const now = Date.now();
                
                // Skip if same message was logged within last 2 seconds
                if (this.loggedMessages.has(messageKey)) {
                    return;
                }
                
                // Add to logged messages and clean up old entries
                this.loggedMessages.add(messageKey);
                setTimeout(() => this.loggedMessages.delete(messageKey), 2000);
                
                const logContainer = this.controlPanel.querySelector('.aa-log');
                const logEntry = document.createElement('div');
                logEntry.className = `aa-log-entry ${type}`;
                logEntry.textContent = `${new Date().toLocaleTimeString()}: ${message}`;
                
                logContainer.appendChild(logEntry);
                logContainer.scrollTop = logContainer.scrollHeight;
                
                // Keep only last 20 entries
                while (logContainer.children.length > 20) {
                    logContainer.removeChild(logContainer.firstChild);
                }
            }
            
            showControlPanel() {
                if (!this.controlPanel) this.createControlPanel();
                this.controlPanel.style.display = 'block';
            }
            
            hideControlPanel() {
                if (this.controlPanel) {
                    this.controlPanel.style.display = 'none';
                }
            }
            
            log(message) {
                const timestamp = new Date().toISOString();
                const prefix = '[AutoAccept]';
                const fullMessage = `${prefix} ${timestamp} - ${message}`;
                
                // Console logging
                console.log(fullMessage);
                
                // Panel logging
                this.logToPanel(message, 'info');
            }
            
            // Find the input box and check its previous siblings for buttons
            findAcceptButtons() {
                const buttons = [];
                
                // ALWAYS search globally first for maximum coverage
                const globalButtons = this.findButtonsGlobally();
                buttons.push(...globalButtons);
                
                // IDE-specific input box selectors
                let inputBox = null;
                
                if (this.ideType === 'windsurf') {
                    // Windsurf doesn't have a specific input box, look for button containers directly
                    inputBox = document.querySelector('.flex.w-full.items-center.justify-between') ||
                              document.querySelector('[class*="bg-ide-editor-background"]') ||
                              document.querySelector('.flex.flex-row.gap-x-1');
                } else {
                    // Cursor IDE
                    inputBox = document.querySelector('div.full-input-box');
                }
                
                if (inputBox) {
                    if (this.ideType === 'windsurf') {
                        // For Windsurf, search the entire document for button patterns
                        const windsurfButtons = this.findWindsurfButtons();
                        buttons.push(...windsurfButtons);
                    } else {
                        // Cursor IDE - Enhanced search strategy
                        // 1. Check previous sibling elements for regular buttons
                        let currentElement = inputBox.previousElementSibling;
                        let searchDepth = 0;
                        
                        while (currentElement && searchDepth < 15) { // Increased search depth
                            // Look for any clickable elements containing button text
                            const acceptElements = this.findAcceptInElement(currentElement);
                            buttons.push(...acceptElements);
                            
                            currentElement = currentElement.previousElementSibling;
                            searchDepth++;
                        }
                        
                        // 2. Search in conversation area for buttons that might appear after Accept
                        const conversationArea = document.querySelector('div.conversations') || 
                                              document.querySelector('[class*="conversation"]') ||
                                              document.querySelector('[class*="chat"]');
                        
                        if (conversationArea) {
                            const conversationButtons = this.findButtonsInConversation(conversationArea);
                            buttons.push(...conversationButtons);
                        }
                        
                        // 3. Search in any visible modal or overlay that might contain buttons
                        const modals = document.querySelectorAll('[class*="modal"], [class*="overlay"], [class*="popup"], [class*="dialog"]');
                        for (const modal of modals) {
                            if (this.isElementVisible(modal)) {
                                const modalButtons = this.findAcceptInElement(modal);
                                buttons.push(...modalButtons);
                            }
                        }
                    }
                }

                // Also search for Resume Conversation links in message bubbles if enabled
                if (this.config.enableResume) {
                    const resumeLinks = this.findResumeLinks();
                    buttons.push(...resumeLinks);
                }

                // Search for connection failure buttons (Resume/Try again in dropdowns)
                if (this.config.enableConnectionResume || this.config.enableTryAgain) {
                    const connectionButtons = this.findConnectionFailureButtons();
                    buttons.push(...connectionButtons);
                }
                
                // Remove duplicates and filter out invalid buttons
                const uniqueButtons = this.removeDuplicateButtons(buttons);
                
                if (this.debugMode && uniqueButtons.length > 0) {
                    this.log(`Found ${uniqueButtons.length} buttons: ${uniqueButtons.map(b => b.textContent.trim().substring(0, 20)).join(', ')}`);
                }
                
                return uniqueButtons;
            }
            
            // Find buttons in conversation area (for buttons that appear after Accept)
            findButtonsInConversation(conversationArea) {
                const buttons = [];
                
                // Look for buttons in the latest message bubbles
                const messageBubbles = Array.from(conversationArea.querySelectorAll('[data-message-index]'))
                    .sort((a, b) => {
                        const indexA = parseInt(a.getAttribute('data-message-index'));
                        const indexB = parseInt(b.getAttribute('data-message-index'));
                        return indexB - indexA; // Latest first
                    });
                
                // Check the latest 3 message bubbles for buttons
                for (let i = 0; i < Math.min(3, messageBubbles.length); i++) {
                    const bubble = messageBubbles[i];
                    const bubbleButtons = this.findAcceptInElement(bubble);
                    buttons.push(...bubbleButtons);
                }
                
                return buttons;
            }
            
            // Remove duplicate buttons based on element reference
            removeDuplicateButtons(buttons) {
                const seen = new Set();
                return buttons.filter(button => {
                    if (seen.has(button)) {
                        return false;
                    }
                    seen.add(button);
                    return true;
                });
            }
            
            // Find accept buttons within a specific element
            findAcceptInElement(element) {
                const buttons = [];
                
                // Get all clickable elements (divs, buttons, spans with click handlers)
                const clickableSelectors = [
                    'div[class*="button"]',
                    'button',
                    'div[onclick]',
                    'div[style*="cursor: pointer"]',
                    'div[style*="cursor:pointer"]',
                    '[class*="anysphere"]',
                    '[class*="cursor-button"]',
                    '[class*="text-button"]',
                    '[class*="primary-button"]',
                    '[class*="secondary-button"]',
                    // Additional selectors for Cursor IDE
                    'span[class*="button"]',
                    'span[style*="cursor: pointer"]',
                    'a[class*="button"]',
                    'div[role="button"]',
                    'span[role="button"]',
                    // Selectors for file review buttons
                    '[class*="review"]',
                    '[class*="file"]',
                    '[class*="accept"]',
                    '[class*="keep"]',
                    '[class*="run"]',
                    '[class*="apply"]',
                    '[class*="execute"]',
                    // Enhanced selectors for better coverage
                    '[data-testid*="button"]',
                    '[aria-label*="accept"]',
                    '[aria-label*="keep"]',
                    '[aria-label*="run"]',
                    '[aria-label*="apply"]',
                    '[aria-label*="execute"]',
                    '[title*="accept"]',
                    '[title*="keep"]',
                    '[title*="run"]',
                    '[title*="apply"]',
                    '[title*="execute"]',
                    // Generic clickable elements
                    '[onclick]',
                    '[style*="cursor: pointer"]',
                    '[style*="cursor:pointer"]',
                    // Cursor-specific patterns
                    '.anysphere-text-button',
                    '.anysphere-secondary-button',
                    '.anysphere-primary-button',
                    // File change buttons
                    '[class*="file-change"]',
                    '[class*="diff"]',
                    '[class*="code-block"]'
                ];
                
                for (const selector of clickableSelectors) {
                    const elements = element.querySelectorAll(selector);
                    for (const el of elements) {
                        if (this.isAcceptButton(el)) {
                            buttons.push(el);
                        }
                    }
                }
                
                // Also check the element itself
                if (this.isAcceptButton(element)) {
                    buttons.push(element);
                }
                
                return buttons;
            }
            
            // Check if element is an Accept button
            isAcceptButton(element) {
                if (!element || !element.textContent) return false;

                // Check if it's a Resume Conversation link first
                if (this.config.enableResume && this.isResumeLink(element)) {
                    return true;
                }
                
                // Use IDE-specific detection
                if (this.ideType === 'windsurf') {
                    return this.isWindsurfAcceptButton(element);
                }
                
                // Cursor IDE detection (original logic)
                const text = element.textContent.toLowerCase().trim();
                
                // Check each pattern based on configuration
                const patterns = [
                    { pattern: 'accept all', enabled: this.config.enableAcceptAll },
                    { pattern: 'accept', enabled: this.config.enableAccept },
                    { pattern: 'review next file', enabled: this.config.enableReviewNextFile },
                    { pattern: 'keep all', enabled: this.config.enableKeepAll },
                    { pattern: 'keep', enabled: this.config.enableKeep },
                    { pattern: 'run command', enabled: this.config.enableRunCommand },
                    { pattern: 'run', enabled: this.config.enableRun },
                    { pattern: 'apply', enabled: this.config.enableApply },
                    { pattern: 'execute', enabled: this.config.enableExecute },
                    { pattern: 'resume', enabled: this.config.enableResume || this.config.enableConnectionResume },
                    { pattern: 'try again', enabled: this.config.enableTryAgain }
                ];
                
                // Check if text matches any enabled pattern
                const matchesEnabledPattern = patterns.some(({ pattern, enabled }) => 
                    enabled && text.includes(pattern)
                );
                
                if (!matchesEnabledPattern) return false;
                
                const isVisible = this.isElementVisible(element);
                const isClickable = this.isElementClickable(element);
                
                return isVisible && isClickable;
            }
            
            // Check if element is visible
            isElementVisible(element) {
                const style = window.getComputedStyle(element);
                const rect = element.getBoundingClientRect();
                
                return (
                    style.display !== 'none' &&
                    style.visibility !== 'hidden' &&
                    parseFloat(style.opacity) > 0.1 &&
                    rect.width > 0 &&
                    rect.height > 0
                );
            }
            
            // Check if element is clickable
            isElementClickable(element) {
                const style = window.getComputedStyle(element);
                return (
                    style.pointerEvents !== 'none' &&
                    !element.disabled &&
                    !element.hasAttribute('disabled')
                );
            }
            
            // Click element with multiple strategies
            clickElement(element) {
                try {
                    // Determine button type for better tracking
                    const buttonText = element.textContent.trim().toLowerCase();
                    const isResumeLink = this.isResumeLink(element);
                    
                    if (this.debugMode) {
                        this.log(`=== DEBUG: clickElement called ===`);
                        this.log(`Button text: "${buttonText}"`);
                        this.log(`Is Resume Link: ${isResumeLink}`);
                        this.log(`Element classes: ${element.className}`);
                        this.log(`Element tag: ${element.tagName}`);
                    }

                    // Extract file info before clicking (only for non-resume buttons)
                    let fileInfo = null;
                    if (!isResumeLink) {
                        fileInfo = this.extractFileInfo(element);
                        if (this.debugMode) {
                            this.log(`DEBUG: File info extraction result: ${fileInfo ? JSON.stringify(fileInfo) : 'null'}`);
                        }
                    }
                    
                    const rect = element.getBoundingClientRect();
                    const x = rect.left + rect.width / 2;
                    const y = rect.top + rect.height / 2;
                    
                    if (this.debugMode) {
                        this.log(`DEBUG: Element position: x=${x}, y=${y}`);
                    }
                    
                    // Extra event sequence: some Windsurf UI elements (e.g., "Run command" Accept)
                    // listen to pointer or mouse down/up rather than a plain click. Dispatch a
                    // full down → up cycle before our standard click strategy to guarantee the
                    // event handler stack is triggered across all IDE variants.
                    try {
                        const pointerDown = new PointerEvent('pointerdown', {
                            bubbles: true,
                            cancelable: true,
                            view: window,
                            clientX: x,
                            clientY: y,
                            pointerType: 'mouse'
                        });
                        element.dispatchEvent(pointerDown);
                    } catch (_) {
                        // PointerEvent may not be supported in some environments – safe to ignore.
                    }
                    const mouseDown = new MouseEvent('mousedown', {
                        bubbles: true,
                        cancelable: true,
                        view: window,
                        clientX: x,
                        clientY: y
                    });
                    element.dispatchEvent(mouseDown);
                    
                    // Strategy 1: Direct click
                    element.click();
                    
                    // Strategy 2: Mouse events
                    const mouseEvent = new MouseEvent('click', {
                        bubbles: true,
                        cancelable: true,
                        view: window,
                        clientX: x,
                        clientY: y
                    });
                    element.dispatchEvent(mouseEvent);
                    
                    // Complete the event cycle with up events to mirror real user interaction.
                    const mouseUp = new MouseEvent('mouseup', {
                        bubbles: true,
                        cancelable: true,
                        view: window,
                        clientX: x,
                        clientY: y
                    });
                    element.dispatchEvent(mouseUp);
                    try {
                        const pointerUp = new PointerEvent('pointerup', {
                            bubbles: true,
                            cancelable: true,
                            view: window,
                            clientX: x,
                            clientY: y,
                            pointerType: 'mouse'
                        });
                        element.dispatchEvent(pointerUp);
                    } catch (_) {
                        // Ignore if PointerEvent unsupported.
                    }
                    
                    // Strategy 3: Focus and Enter (retain original behaviour)
                    if (element.focus) element.focus();
                    const enterEvent = new KeyboardEvent('keydown', {
                        key: 'Enter',
                        code: 'Enter',
                        keyCode: 13,
                        bubbles: true
                    });
                    element.dispatchEvent(enterEvent);
                    
                    // Handle different button types for analytics
                    if (isResumeLink) {
                        // For Resume Conversation links, just track the action
                        const timeSaved = this.calculateTimeSaved('resume-conversation');
                        this.logToPanel(`🔄 Resume Conversation clicked [saved ${this.formatTimeDuration(timeSaved)}]`, 'info');
                        this.log(`Resume Conversation clicked - Time saved: ${this.formatTimeDuration(timeSaved)}`);
                        
                        // Track button type count
                        if (!this.analytics.buttonTypeCounts) {
                            this.analytics.buttonTypeCounts = {};
                        }
                        this.analytics.buttonTypeCounts['resume-conversation'] = (this.analytics.buttonTypeCounts['resume-conversation'] || 0) + 1;
                        
                        // Update totals
                        this.analytics.totalAccepts++;
                        this.roiTracking.totalTimeSaved += timeSaved;
                        
                        // Save to storage
                        this.saveToStorage();
                    } else if (buttonText === 'resume' && !isResumeLink) {
                        // Handle connection failure Resume button
                        const timeSaved = this.calculateTimeSaved('connection-resume');
                        this.logToPanel(`🔄 Connection Resume clicked [saved ${this.formatTimeDuration(timeSaved)}]`, 'info');
                        this.log(`Connection Resume clicked - Time saved: ${this.formatTimeDuration(timeSaved)}`);
                        
                        // Track button type count
                        if (!this.analytics.buttonTypeCounts) {
                            this.analytics.buttonTypeCounts = {};
                        }
                        this.analytics.buttonTypeCounts['connection-resume'] = (this.analytics.buttonTypeCounts['connection-resume'] || 0) + 1;
                        
                        // Update totals
                        this.analytics.totalAccepts++;
                        this.roiTracking.totalTimeSaved += timeSaved;
                        
                        // Save to storage
                        this.saveToStorage();
                    } else if (buttonText === 'try again') {
                        // Handle Try again button
                        const timeSaved = this.calculateTimeSaved('try again');
                        this.logToPanel(`🔄 Try Again clicked [saved ${this.formatTimeDuration(timeSaved)}]`, 'info');
                        this.log(`Try Again clicked - Time saved: ${this.formatTimeDuration(timeSaved)}`);
                        
                        // Track button type count
                        if (!this.analytics.buttonTypeCounts) {
                            this.analytics.buttonTypeCounts = {};
                        }
                        this.analytics.buttonTypeCounts['try-again'] = (this.analytics.buttonTypeCounts['try-again'] || 0) + 1;
                        
                        // Update totals
                        this.analytics.totalAccepts++;
                        this.roiTracking.totalTimeSaved += timeSaved;
                        
                        // Save to storage
                        this.saveToStorage();
                    } else if (fileInfo) {
                        // Track file analytics for regular buttons
                        this.trackFileAcceptance(fileInfo, buttonText);
                    } else {
                        // Still track time saved even without file info
                        const timeSaved = this.calculateTimeSaved(buttonText);
                        this.logToPanel(`✓ Clicked: ${element.textContent.trim()} [saved ${this.formatTimeDuration(timeSaved)}]`, 'info');
                        
                        // Track button type count
                        const normalizedType = this.normalizeButtonType(buttonText);
                        if (!this.analytics.buttonTypeCounts) {
                            this.analytics.buttonTypeCounts = {};
                        }
                        this.analytics.buttonTypeCounts[normalizedType] = (this.analytics.buttonTypeCounts[normalizedType] || 0) + 1;
                        
                        // Update totals
                        this.analytics.totalAccepts++;
                        this.roiTracking.totalTimeSaved += timeSaved;
                        
                        // Save to storage
                        this.saveToStorage();
                    }
                    
                    // Update UI
                    this.updatePanelStatus();
                    if (this.currentTab === 'analytics' || this.currentTab === 'roi') {
                        this.updateAnalyticsContent();
                    }
                    this.updateMainFooter();
                    
                    return true;
                } catch (error) {
                    this.logToPanel(`Failed to click: ${error.message}`, 'warning');
                    if (this.debugMode) {
                        this.log(`DEBUG: Click error stack: ${error.stack}`);
                    }
                    return false;
                }
            }
            
            // Main execution
            checkAndClick() {
                try {
                    const buttons = this.findAcceptButtons();
                    
                    if (buttons.length === 0) {
                        // Don't spam the log for "no buttons found"
                        return;
                    }
                    
                    // Enhanced button clicking logic with priority and sequence handling
                    const clickedButton = this.clickButtonsInSequence(buttons);
                    
                    if (clickedButton) {
                        this.totalClicks++;
                        this.updatePanelStatus();
                        
                        // Log the action
                        const buttonText = clickedButton.textContent.trim().substring(0, 30);
                        this.logToPanel(`✓ Clicked: ${buttonText}`, 'info');
                        
                        // After clicking, immediately search for new buttons that might have appeared
                        // This handles the Accept -> Review next file -> Keep sequence
                        setTimeout(() => {
                            this.findAndClickNextButtons();
                        }, 500); // Wait 500ms for UI to update
                    }
                    
                } catch (error) {
                    this.log(`Error executing: ${error.message}`);
                }
            }
            
            // Find and click next buttons that appear after a click
            findAndClickNextButtons() {
                try {
                    const newButtons = this.findAcceptButtons();
                    
                    if (newButtons.length === 0) {
                        return;
                    }
                    
                    // Click the highest priority button
                    const clickedButton = this.clickButtonsInSequence(newButtons);
                    
                    if (clickedButton) {
                        this.totalClicks++;
                        this.updatePanelStatus();
                        
                        const buttonText = clickedButton.textContent.trim().substring(0, 30);
                        this.logToPanel(`✓ Next: ${buttonText}`, 'info');
                        
                        // Continue searching for more buttons recursively
                        setTimeout(() => {
                            this.findAndClickNextButtons();
                        }, 500);
                    }
                    
                } catch (error) {
                    this.log(`Error in findAndClickNextButtons: ${error.message}`);
                }
            }
            
            // Enhanced button clicking with priority and sequence handling
            clickButtonsInSequence(buttons) {
                // Sort buttons by priority: Accept -> Review next file -> Keep -> Run -> Apply -> Execute
                const priorityOrder = [
                    'accept all',
                    'accept', 
                    'review next file',
                    'keep all',
                    'keep',
                    'run command',
                    'run',
                    'apply',
                    'execute',
                    'resume',
                    'try again'
                ];
                
                // Sort buttons by priority
                const sortedButtons = buttons.sort((a, b) => {
                    const textA = a.textContent.toLowerCase().trim();
                    const textB = b.textContent.toLowerCase().trim();
                    
                    const priorityA = priorityOrder.findIndex(pattern => textA.includes(pattern));
                    const priorityB = priorityOrder.findIndex(pattern => textB.includes(pattern));
                    
                    // If both have priority, sort by priority order
                    if (priorityA !== -1 && priorityB !== -1) {
                        return priorityA - priorityB;
                    }
                    
                    // If only one has priority, prioritize it
                    if (priorityA !== -1) return -1;
                    if (priorityB !== -1) return 1;
                    
                    // If neither has priority, maintain original order
                    return 0;
                });
                
                // Try to click buttons in priority order
                for (const button of sortedButtons) {
                    if (this.isElementVisible(button) && this.isElementClickable(button)) {
                        const success = this.clickElement(button);
                        if (success) {
                            return button;
                        }
                    }
                }
                
                return null;
            }
            
            start() {
                if (this.isRunning) {
                    this.logToPanel('Already running', 'warning');
                    return;
                }
                
                this.isRunning = true;
                this.totalClicks = 0;
                this.updatePanelStatus();
                
                // Initial check
                this.checkAndClick();
                
                // Set interval
                this.monitorInterval = setInterval(() => {
                    this.checkAndClick();
                }, this.interval);
                
                this.logToPanel(`Started (${this.interval/1000}s interval)`, 'info');
            }
            
            stop() {
                if (!this.isRunning) {
                    this.logToPanel('Not running', 'warning');
                    return;
                }
                
                clearInterval(this.monitorInterval);
                this.isRunning = false;
                this.updatePanelStatus();
                this.logToPanel(`Stopped (${this.totalClicks} clicks)`, 'info');
            }
            
            status() {
                return {
                    isRunning: this.isRunning,
                    interval: this.interval,
                    totalClicks: this.totalClicks,
                    config: this.config
                };
            }
            
            // Configuration control methods
            enableOnly(buttonTypes) {
                // Disable all first
                Object.keys(this.config).forEach(key => {
                    this.config[key] = false;
                });
                
                // Enable specified types
                buttonTypes.forEach(type => {
                    const configKey = `enable${type.charAt(0).toUpperCase() + type.slice(1)}`;
                    if (this.config.hasOwnProperty(configKey)) {
                        this.config[configKey] = true;
                        this.log(`Enabled ${type} buttons`);
                    }
                });
                
                this.log(`Configuration updated: Only ${buttonTypes.join(', ')} buttons enabled`);
            }
            
            enableAll() {
                Object.keys(this.config).forEach(key => {
                    this.config[key] = true;
                });
                this.log('All button types enabled');
            }
            
            disableAll() {
                Object.keys(this.config).forEach(key => {
                    this.config[key] = false;
                });
                this.log('All button types disabled');
            }
            
            toggle(buttonType) {
                const configKey = `enable${buttonType.charAt(0).toUpperCase() + buttonType.slice(1)}`;
                if (this.config.hasOwnProperty(configKey)) {
                    this.config[configKey] = !this.config[configKey];
                    this.log(`${buttonType} buttons ${this.config[configKey] ? 'enabled' : 'disabled'}`);
                } else {
                    this.log(`Unknown button type: ${buttonType}`);
                }
            }
            
            enable(buttonType) {
                const configKey = `enable${buttonType.charAt(0).toUpperCase() + buttonType.slice(1)}`;
                if (this.config.hasOwnProperty(configKey)) {
                    this.config[configKey] = true;
                    this.log(`${buttonType} buttons enabled`);
                } else {
                    this.log(`Unknown button type: ${buttonType}`);
                }
            }
            
            disable(buttonType) {
                const configKey = `enable${buttonType.charAt(0).toUpperCase() + buttonType.slice(1)}`;
                if (this.config.hasOwnProperty(configKey)) {
                    this.config[configKey] = false;
                    this.log(`${buttonType} buttons disabled`);
                } else {
                    this.log(`Unknown button type: ${buttonType}`);
                }
            }
            
            // Manual search for debugging
            debugSearch() {
                this.log('=== DEBUG SEARCH ===');
                const inputBox = document.querySelector('div.full-input-box');
                if (!inputBox) {
                    this.log('No input box found');
                    return;
                }
                
                this.log('Input box found, checking siblings...');
                
                let currentElement = inputBox.previousElementSibling;
                let siblingIndex = 1;
                
                while (currentElement && siblingIndex <= 10) {
                    this.log(`Sibling ${siblingIndex}: ${currentElement.tagName} ${currentElement.className}`);
                    
                    // Check for any text content
                    const text = currentElement.textContent ? currentElement.textContent.trim() : '';
                    if (text) {
                        this.log(`  Text: "${text.substring(0, 100)}"`);
                        
                        // Check specifically for run/accept patterns
                        const patterns = ['accept', 'run', 'execute', 'apply'];
                        const foundPatterns = patterns.filter(pattern => text.toLowerCase().includes(pattern));
                        if (foundPatterns.length > 0) {
                            this.log(`  >>> Contains patterns: ${foundPatterns.join(', ')}`);
                        }
                    }
                    
                    // Check for buttons in this sibling
                    const buttons = this.findAcceptInElement(currentElement);
                    if (buttons.length > 0) {
                        this.log(`  Found ${buttons.length} clickable buttons!`);
                        buttons.forEach((btn, i) => {
                            this.log(`    Button ${i+1}: "${btn.textContent.trim().substring(0, 50)}"`);
                        });
                    }
                    
                    currentElement = currentElement.previousElementSibling;
                    siblingIndex++;
                }
                
                this.log('=== END DEBUG ===');
            }
            
            // Find Resume Conversation links in message bubbles
            findResumeLinks() {
                const resumeLinks = [];
                
                // Look for Resume Conversation markdown links
                const resumeSelectors = [
                    '.markdown-link[data-link="command:composer.resumeCurrentChat"]',
                    '.markdown-link[data-link*="resume"]',
                    'span.markdown-link[data-link="command:composer.resumeCurrentChat"]'
                ];
                
                for (const selector of resumeSelectors) {
                    const elements = document.querySelectorAll(selector);
                    for (const element of elements) {
                        if (this.isElementVisible(element) && this.isElementClickable(element)) {
                            resumeLinks.push(element);
                        }
                    }
                }
                
                return resumeLinks;
            }

            // New method: Find connection failure buttons (Resume/Try again in dropdowns)
            findConnectionFailureButtons() {
                const buttons = [];
                
                // Look for connection failure dropdown containers
                const dropdownSelectors = [
                    '.bg-dropdown-background',
                    '[class*="dropdown"]',
                    '[class*="fade-in"]'
                ];
                
                for (const selector of dropdownSelectors) {
                    const dropdowns = document.querySelectorAll(selector);
                    
                    for (const dropdown of dropdowns) {
                        // Check if this dropdown contains connection failure text
                        const text = dropdown.textContent.toLowerCase();
                        if (text.includes('connection failed') || 
                            text.includes('check your internet') || 
                            text.includes('vpn')) {
                            
                            // Look for Resume and Try again buttons within this dropdown
                            const buttonSelectors = [
                                '.anysphere-secondary-button',
                                '.anysphere-text-button',
                                '[class*="button"]',
                                '[style*="cursor: pointer"]'
                            ];
                            
                            for (const btnSelector of buttonSelectors) {
                                const dropdownButtons = dropdown.querySelectorAll(btnSelector);
                                
                                for (const btn of dropdownButtons) {
                                    const btnText = btn.textContent.toLowerCase().trim();
                                    
                                    // Check for Resume or Try again buttons
                                    if ((btnText === 'resume' && this.config.enableConnectionResume) ||
                                        (btnText === 'try again' && this.config.enableTryAgain)) {
                                        
                                        if (this.isElementVisible(btn) && this.isElementClickable(btn)) {
                                            buttons.push(btn);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                return buttons;
            }

            // Check if element is a Resume Conversation link
            isResumeLink(element) {
                if (!element) return false;
                
                // Check for Resume Conversation specific attributes and text
                const hasResumeCommand = element.getAttribute('data-link') === 'command:composer.resumeCurrentChat';
                const hasResumeText = element.textContent && element.textContent.toLowerCase().includes('resume');
                const isMarkdownLink = element.classList.contains('markdown-link');
                
                if (!hasResumeCommand && !hasResumeText) return false;
                
                const isVisible = this.isElementVisible(element);
                const isClickable = this.isElementClickable(element);
                
                return isVisible && isClickable;
            }

            // Diff Block Detection and Analysis
            findDiffBlocks() {
                const diffBlocks = [];
                
                // Look for composer diff blocks in the conversation
                const diffSelectors = [
                    'div.composer-diff-block',
                    'div.composer-code-block-container',
                    'div.composer-tool-former-message'
                ];
                
                for (const selector of diffSelectors) {
                    const blocks = document.querySelectorAll(selector);
                    for (const block of blocks) {
                        const diffInfo = this.analyzeDiffBlock(block);
                        if (diffInfo) {
                            diffBlocks.push(diffInfo);
                        }
                    }
                }
                
                return diffBlocks;
            }

            // Analyze a single diff block for file information
            analyzeDiffBlock(block) {
                try {
                    if (!block) return null;
                    
                    const diffInfo = {
                        blockElement: block,
                        timestamp: new Date(),
                        files: [],
                        changeType: 'unknown'
                    };
                    
                    // Look for file header information
                    const fileHeader = block.querySelector('.composer-code-block-header');
                    if (fileHeader) {
                        const fileInfo = this.extractFileInfoFromHeader(fileHeader);
                        if (fileInfo) {
                            diffInfo.files.push(fileInfo);
                        }
                    }
                    
                    // Look for file name in the filename span
                    const filenameSpan = block.querySelector('.composer-code-block-filename span');
                    if (filenameSpan && !diffInfo.files.length) {
                        const filename = filenameSpan.textContent.trim();
                        if (filename) {
                            diffInfo.files.push({
                                name: filename,
                                path: filename,
                                extension: this.getFileExtension(filename)
                            });
                        }
                    }
                    
                    // Check for change indicators (+/- numbers)
                    const statusSpan = block.querySelector('.composer-code-block-status span[style*="color"]');
                    if (statusSpan) {
                        const statusText = statusSpan.textContent.trim();
                        if (statusText.includes('+')) {
                            diffInfo.changeType = 'addition';
                            diffInfo.linesAdded = this.extractNumber(statusText);
                        } else if (statusText.includes('-')) {
                            diffInfo.changeType = 'deletion';
                            diffInfo.linesDeleted = this.extractNumber(statusText);
                        }
                    }
                    
                    // Look for both additions and deletions
                    const allStatusSpans = block.querySelectorAll('.composer-code-block-status span[style*="color"]');
                    let hasAdditions = false, hasDeletions = false;
                    
                    allStatusSpans.forEach(span => {
                        const text = span.textContent.trim();
                        if (text.includes('+')) {
                            hasAdditions = true;
                            diffInfo.linesAdded = this.extractNumber(text);
                        } else if (text.includes('-')) {
                            hasDeletions = true;
                            diffInfo.linesDeleted = this.extractNumber(text);
                        }
                    });
                    
                    if (hasAdditions && hasDeletions) {
                        diffInfo.changeType = 'modification';
                    } else if (hasAdditions) {
                        diffInfo.changeType = 'addition';
                    } else if (hasDeletions) {
                        diffInfo.changeType = 'deletion';
                    }
                    
                    return diffInfo.files.length > 0 ? diffInfo : null;
                    
                } catch (error) {
                    this.log(`Error analyzing diff block: ${error.message}`);
                    return null;
                }
            }

            // Extract file information from code block header
            extractFileInfoFromHeader(header) {
                try {
                    const fileInfo = header.querySelector('.composer-code-block-file-info');
                    if (!fileInfo) return null;
                    
                    const filenameElement = fileInfo.querySelector('.composer-code-block-filename span');
                    const filename = filenameElement ? filenameElement.textContent.trim() : null;
                    
                    if (!filename) return null;
                    
                    return {
                        name: filename,
                        path: filename,
                        extension: this.getFileExtension(filename),
                        hasIcon: !!fileInfo.querySelector('.composer-code-block-file-icon')
                    };
                    
                } catch (error) {
                    this.log(`Error extracting file info from header: ${error.message}`);
                    return null;
                }
            }

            // Get file extension from filename
            getFileExtension(filename) {
                if (!filename || typeof filename !== 'string') return '';
                const lastDot = filename.lastIndexOf('.');
                return lastDot > 0 ? filename.substring(lastDot + 1).toLowerCase() : '';
            }

            // Extract numbers from text (e.g., "+17" -> 17)
            extractNumber(text) {
                if (!text) return 0;
                const match = text.match(/[+-]?(\d+)/);
                return match ? parseInt(match[1], 10) : 0;
            }

            // Find recent diff blocks in conversation
            findRecentDiffBlocks(maxAge = 30000) { // 30 seconds by default
                const allDiffs = this.findDiffBlocks();
                const cutoffTime = Date.now() - maxAge;
                
                return allDiffs.filter(diff => 
                    diff.timestamp && diff.timestamp.getTime() > cutoffTime
                );
            }

            // Get conversation context for file changes
            getConversationContext() {
                const conversationDiv = document.querySelector('div.conversations');
                if (!conversationDiv) {
                    this.log('Conversation container not found');
                    return null;
                }
                
                const context = {
                    conversationElement: conversationDiv,
                    totalMessages: 0,
                    recentDiffs: [],
                    filesChanged: new Set(),
                    lastActivity: null
                };
                
                // Count message bubbles
                const messageBubbles = conversationDiv.querySelectorAll('[data-message-index]');
                context.totalMessages = messageBubbles.length;
                
                // Find recent diff blocks
                const recentDiffs = this.findRecentDiffBlocks();
                context.recentDiffs = recentDiffs;
                
                // Extract unique files from recent diffs
                recentDiffs.forEach(diff => {
                    diff.files.forEach(file => {
                        context.filesChanged.add(file.name);
                    });
                });
                
                // Convert Set to Array for easier handling
                context.filesChanged = Array.from(context.filesChanged);
                
                // Find last activity timestamp
                if (messageBubbles.length > 0) {
                    const lastBubble = messageBubbles[messageBubbles.length - 1];
                    context.lastActivity = new Date(); // Current time as approximation
                }
                
                return context;
            }

            // Enhanced logging with conversation context
            logConversationActivity() {
                const context = this.getConversationContext();
                if (!context) return;
                
                this.log('=== CONVERSATION ACTIVITY ===');
                this.log(`Total messages: ${context.totalMessages}`);
                this.log(`Recent diffs: ${context.recentDiffs.length}`);
                this.log(`Files changed: ${context.filesChanged.length}`);
                
                if (context.filesChanged.length > 0) {
                    this.log(`Changed files: ${context.filesChanged.join(', ')}`);
                }
                
                context.recentDiffs.forEach((diff, index) => {
                    this.log(`Diff ${index + 1}: ${diff.changeType} - ${diff.files.map(f => f.name).join(', ')}`);
                    if (diff.linesAdded) this.log(`  +${diff.linesAdded} lines added`);
                    if (diff.linesDeleted) this.log(`  -${diff.linesDeleted} lines deleted`);
                });
                
                this.log('=== END CONVERSATION ACTIVITY ===');
            }

            // Windsurf-specific button detection
            findWindsurfButtons() {
                const buttons = [];
                
                // Search in main document first
                this.searchWindsurfButtonsInDocument(document, buttons);
                
                // Search in iframes (Windsurf often runs in iframe)
                const iframes = document.querySelectorAll('iframe');
                for (const iframe of iframes) {
                    try {
                        // Check if this is the Windsurf iframe
                        if (iframe.id === 'windsurf.cascadePanel' || 
                            iframe.src.includes('windsurf') || 
                            iframe.src.includes('cascadePanel')) {
                            
                            if (this.debugMode) {
                                this.log(`DEBUG: Found Windsurf iframe: ${iframe.id || iframe.src}`);
                            }
                            
                            // Access iframe document
                            const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
                            if (iframeDoc) {
                                this.searchWindsurfButtonsInDocument(iframeDoc, buttons);
                                if (this.debugMode) {
                                    this.log(`DEBUG: Searched iframe document, found ${buttons.length} total buttons so far`);
                                }
                            }
                        }
                    } catch (error) {
                        // Cross-origin or access restrictions
                        if (this.debugMode) {
                            this.log(`DEBUG: Cannot access iframe content: ${error.message}`);
                        }
                    }
                }
                
                return buttons;
            }
            
            // Helper method to search for Windsurf buttons in a specific document
            searchWindsurfButtonsInDocument(doc, buttons) {
                // Windsurf button selectors based on the provided HTML
                const windsurfSelectors = [
                    // Accept/Reject buttons in command execution
                    'button.hover\\:bg-ide-button-hover-background.cursor-pointer.rounded.bg-ide-button-background',
                    'button[class*="bg-ide-button-background"]',
                    'button[class*="text-ide-button-color"]',
                    
                    // Accept all buttons in file changes (enhanced for the specific UI structure)
                    'span.hover\\:text-ide-button-hover-color.hover\\:bg-ide-button-hover-background.cursor-pointer',
                    'span[class*="bg-ide-button-background"]',
                    'span[class*="text-ide-button-color"]',
                    'span.hover\\:bg-ide-button-hover-background.cursor-pointer.select-none.rounded-sm.bg-ide-button-background',
                    
                    // Generic clickable elements with specific Windsurf patterns
                    '[class*="cursor-pointer"][class*="rounded"]',
                    '[class*="cursor-pointer"][class*="select-none"]',
                    'button[class*="transition"]',
                    'span[class*="cursor-pointer"]',
                    
                    // More specific selectors for file changes UI
                    'span.cursor-pointer.select-none.rounded-sm',
                    'span[class*="hover:text-ide-button-hover-color"]',
                    'span[class*="hover:bg-ide-button-hover-background"]'
                ];
                
                for (const selector of windsurfSelectors) {
                    try {
                        const elements = doc.querySelectorAll(selector);
                        for (const element of elements) {
                            if (this.isWindsurfAcceptButton(element)) {
                                buttons.push(element);
                                if (this.debugMode) {
                                    this.log(`DEBUG: Found Windsurf button in ${doc === document ? 'main document' : 'iframe'}: "${element.textContent.trim()}"`);
                                }
                            }
                        }
                    } catch (error) {
                        // Skip invalid selectors
                        if (this.debugMode) {
                            this.log(`Windsurf selector error: ${selector} - ${error.message}`);
                        }
                    }
                }
            }

            // Check if element is a Windsurf accept button
            isWindsurfAcceptButton(element) {
                if (!element || !element.textContent) return false;
                
                const text = element.textContent.toLowerCase().trim();
                
                // Windsurf button patterns (enhanced for file changes UI)
                const windsurfPatterns = [
                    { pattern: 'accept all', enabled: this.config.enableAcceptAll },
                    { pattern: 'accept', enabled: this.config.enableAccept },
                    { pattern: 'review next file', enabled: this.config.enableReviewNextFile },
                    { pattern: 'keep all', enabled: this.config.enableKeepAll },
                    { pattern: 'keep', enabled: this.config.enableKeep },
                    { pattern: 'run command', enabled: this.config.enableRunCommand },
                    { pattern: 'run', enabled: this.config.enableRun },
                    { pattern: 'apply', enabled: this.config.enableApply },
                    { pattern: 'execute', enabled: this.config.enableExecute }
                ];
                
                // Check if text matches any enabled pattern
                const matchesPattern = windsurfPatterns.some(({ pattern, enabled }) => 
                    enabled && text.includes(pattern)
                );
                
                if (!matchesPattern) return false;
                
                // Enhanced Windsurf-specific class checks
                const hasWindsurfClasses = element.className.includes('bg-ide-button-background') ||
                                          element.className.includes('text-ide-button-color') ||
                                          element.className.includes('cursor-pointer') ||
                                          element.className.includes('hover:bg-ide-button-hover-background') ||
                                          element.className.includes('hover:text-ide-button-hover-color') ||
                                          element.className.includes('select-none');
                
                if (!hasWindsurfClasses) return false;
                
                // Additional check for reject buttons - exclude them
                if (text.includes('reject')) {
                    if (this.debugMode) {
                        this.log(`DEBUG: Skipping reject button: "${text}"`);
                    }
                    return false;
                }
                
                const isVisible = this.isElementVisible(element);
                const isClickable = this.isElementClickable(element);
                
                if (this.debugMode && matchesPattern) {
                    this.log(`DEBUG: Windsurf button found - Text: "${text}", Classes: "${element.className}", Visible: ${isVisible}, Clickable: ${isClickable}`);
                }
                
                return isVisible && isClickable;
            }

            // Global button search fallback
            findButtonsGlobally() {
                const buttons = [];
                
                // Search in main document
                this.searchButtonsInDocument(document, buttons);
                
                // Search in iframes
                const iframes = document.querySelectorAll('iframe');
                for (const iframe of iframes) {
                    try {
                        const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
                        if (iframeDoc) {
                            this.searchButtonsInDocument(iframeDoc, buttons);
                            if (this.debugMode) {
                                this.log(`DEBUG: Searched iframe ${iframe.id || iframe.src} for global buttons`);
                            }
                        }
                    } catch (error) {
                        // Cross-origin restrictions
                        if (this.debugMode) {
                            this.log(`DEBUG: Cannot access iframe for global search: ${error.message}`);
                        }
                    }
                }
                
                return buttons;
            }
            
            // Helper method to search for buttons in a specific document
            searchButtonsInDocument(doc, buttons) {
                // Combined selectors for both IDEs
                const globalSelectors = [
                    // Cursor selectors
                    'div[class*="button"]',
                    'button',
                    '[class*="anysphere"]',
                    '.anysphere-text-button',
                    '.anysphere-secondary-button',
                    '.anysphere-primary-button',
                    
                    // Windsurf selectors  
                    'button[class*="bg-ide-button-background"]',
                    'span[class*="cursor-pointer"]',
                    '[class*="hover:bg-ide-button-hover-background"]',
                    
                    // Generic selectors
                    '[class*="cursor-pointer"]',
                    '[onclick]',
                    '[style*="cursor: pointer"]',
                    '[style*="cursor:pointer"]',
                    
                    // Enhanced selectors for better coverage
                    '[data-testid*="button"]',
                    '[aria-label*="accept"]',
                    '[aria-label*="keep"]',
                    '[aria-label*="run"]',
                    '[aria-label*="apply"]',
                    '[aria-label*="execute"]',
                    '[title*="accept"]',
                    '[title*="keep"]',
                    '[title*="run"]',
                    '[title*="apply"]',
                    '[title*="execute"]',
                    
                    // File change and review buttons
                    '[class*="file-change"]',
                    '[class*="diff"]',
                    '[class*="code-block"]',
                    '[class*="review"]',
                    '[class*="file"]',
                    '[class*="accept"]',
                    '[class*="keep"]',
                    '[class*="run"]',
                    '[class*="apply"]',
                    '[class*="execute"]',
                    
                    // Role-based selectors
                    '[role="button"]',
                    'div[role="button"]',
                    'span[role="button"]',
                    'a[role="button"]',
                    
                    // Clickable elements
                    'a[class*="button"]',
                    'span[class*="button"]',
                    'div[class*="text-button"]',
                    'div[class*="primary-button"]',
                    'div[class*="secondary-button"]'
                ];
                
                for (const selector of globalSelectors) {
                    try {
                        const elements = doc.querySelectorAll(selector);
                        for (const element of elements) {
                            if (this.isAcceptButton(element)) {
                                buttons.push(element);
                                if (this.debugMode) {
                                    this.log(`DEBUG: Found global button in ${doc === document ? 'main document' : 'iframe'}: "${element.textContent.trim()}"`);
                                }
                            }
                        }
                    } catch (error) {
                        // Skip invalid selectors
                        if (this.debugMode) {
                            this.log(`Global selector error: ${selector} - ${error.message}`);
                        }
                    }
                }
            }
        }
        
        globalThis.autoAcceptAndAnalytics = autoAcceptAndAnalytics;
    }
    
    // Initialize
    if (!globalThis.simpleAccept) {
        globalThis.simpleAccept = new globalThis.autoAcceptAndAnalytics(2000);
        
        // Expose controls
        globalThis.startAccept = () => globalThis.simpleAccept.start();
        globalThis.stopAccept = () => globalThis.simpleAccept.stop();
        globalThis.acceptStatus = () => globalThis.simpleAccept.status();
        globalThis.debugAccept = () => globalThis.simpleAccept.debugSearch();
        
        // Force log test function
        globalThis.testLogs = () => {
            console.log('TEST LOG 1 - console.log');
            console.info('TEST LOG 2 - console.info');
            console.warn('TEST LOG 3 - console.warn');
            console.error('TEST LOG 4 - console.error');
            alert('TEST: Console logging test completed. Check console above.');
            return 'Logging test completed';
        };
        
        // Configuration controls
        globalThis.enableOnly = (types) => globalThis.simpleAccept.enableOnly(types);
        globalThis.enableAll = () => globalThis.simpleAccept.enableAll();
        globalThis.disableAll = () => globalThis.simpleAccept.disableAll();
        globalThis.toggleButton = (type) => globalThis.simpleAccept.toggle(type);
        globalThis.enableButton = (type) => globalThis.simpleAccept.enable(type);
        globalThis.disableButton = (type) => globalThis.simpleAccept.disable(type);
        
        // Analytics controls
        globalThis.exportAnalytics = () => globalThis.simpleAccept.exportAnalytics();
        globalThis.clearAnalytics = () => globalThis.simpleAccept.clearAnalytics();
        globalThis.clearStorage = () => globalThis.simpleAccept.clearStorage();
        globalThis.validateData = () => globalThis.simpleAccept.validateData();
        globalThis.toggleDebug = () => globalThis.simpleAccept.toggleDebug();
        globalThis.calibrateWorkflow = (manualSeconds, autoMs) => globalThis.simpleAccept.calibrateWorkflowTimes(manualSeconds, autoMs);
        globalThis.showAnalytics = () => {
            globalThis.simpleAccept.switchTab('analytics');
            console.log('Analytics tab opened in control panel');
        };
        
        // Conversation Analysis controls
        globalThis.findDiffs = () => globalThis.simpleAccept.findDiffBlocks();
        globalThis.getContext = () => globalThis.simpleAccept.getConversationContext();
        globalThis.logActivity = () => globalThis.simpleAccept.logConversationActivity();
        globalThis.recentDiffs = (maxAge) => globalThis.simpleAccept.findRecentDiffBlocks(maxAge);
        
        // Debug controls
        globalThis.enableDebug = () => {
            globalThis.simpleAccept.debugMode = true;
            console.log('Debug mode enabled - file extraction logging activated');
        };
        globalThis.disableDebug = () => {
            globalThis.simpleAccept.debugMode = false;
            console.log('Debug mode disabled');
        };
        
        // Force visible startup message
        const startupMsg = `[autoAcceptAndAnalytics] SCRIPT LOADED AND ACTIVE! (${globalThis.simpleAccept.ideType.toUpperCase()} IDE detected)`;
        console.log(startupMsg);
        console.info(startupMsg);
        console.warn(startupMsg);
        
        // Also create visual notification
        try {
            const notification = document.createElement('div');
            notification.textContent = `✅ AutoAccept Control Panel Ready! (${globalThis.simpleAccept.ideType.toUpperCase()}) - Click Analytics tab!`;
            notification.style.cssText = 'position:fixed;top:10px;left:50%;transform:translateX(-50%);background:#4CAF50;color:white;padding:10px 20px;border-radius:5px;z-index:99999;font-weight:bold;max-width:400px;text-align:center;';
            document.body.appendChild(notification);
            setTimeout(() => notification.remove(), 4000);
        } catch (e) {
            // Ignore
        }
        
        console.log('Commands: startAccept(), stopAccept(), acceptStatus(), debugAccept()');
        console.log('Analytics: showAnalytics(), exportAnalytics(), clearAnalytics(), clearStorage(), validateData()');
        console.log('Debug: toggleDebug(), enableDebug(), disableDebug() - Control debug logging');
        console.log('Calibration: calibrateWorkflow(manualSeconds, autoMs) - Adjust workflow timing');
        console.log('Config: enableOnly([types]), enableAll(), disableAll(), toggleButton(type)');
        console.log('Conversation: findDiffs(), getContext(), logActivity(), recentDiffs(maxAge)');
        console.log('Types: "acceptAll", "accept", "reviewNextFile", "keepAll", "keep", "run", "runCommand", "apply", "execute", "resume", "connectionResume", "tryAgain"');
    }
})(); 