import React, { useState } from 'react';
import {
  IconButton,
  Tooltip,
  Snackbar,
  Alert,
} from '@mui/material';
import {
  ContentCopy as CopyIcon,
  Check as CheckIcon,
} from '@mui/icons-material';

interface CopyButtonProps {
  text: string;
  tooltip?: string;
  successMessage?: string;
  errorMessage?: string;
  size?: 'small' | 'medium' | 'large';
  color?: 'default' | 'primary' | 'secondary' | 'error' | 'info' | 'success' | 'warning';
  variant?: 'text' | 'outlined' | 'contained';
  disabled?: boolean;
  onCopy?: (text: string) => void;
  onError?: (error: Error) => void;
}

const CopyButton: React.FC<CopyButtonProps> = ({
  text,
  tooltip = 'Copy to clipboard',
  successMessage = 'Copied to clipboard',
  errorMessage = 'Failed to copy',
  size = 'medium',
  color = 'default',
  variant = 'text',
  disabled = false,
  onCopy,
  onError,
}) => {
  const [copied, setCopied] = useState(false);
  const [error, setError] = useState(false);

  const handleCopy = async () => {
    try {
      await navigator.clipboard.writeText(text);
      setCopied(true);
      onCopy?.(text);
    } catch (err) {
      setError(true);
      onError?.(err as Error);
    }
  };

  const handleClose = () => {
    setCopied(false);
    setError(false);
  };

  return (
    <>
      <Tooltip title={tooltip} arrow>
        <IconButton
          onClick={handleCopy}
          disabled={disabled}
          size={size}
          color={color}
          sx={{
            transition: 'all 0.2s ease-in-out',
            '&:hover': {
              transform: 'scale(1.1)',
            },
          }}
        >
          {copied ? <CheckIcon /> : <CopyIcon />}
        </IconButton>
      </Tooltip>

      <Snackbar
        open={copied}
        autoHideDuration={2000}
        onClose={handleClose}
        anchorOrigin={{
          vertical: 'bottom',
          horizontal: 'center',
        }}
      >
        <Alert
          onClose={handleClose}
          severity="success"
          sx={{ width: '100%' }}
        >
          {successMessage}
        </Alert>
      </Snackbar>

      <Snackbar
        open={error}
        autoHideDuration={3000}
        onClose={handleClose}
        anchorOrigin={{
          vertical: 'bottom',
          horizontal: 'center',
        }}
      >
        <Alert
          onClose={handleClose}
          severity="error"
          sx={{ width: '100%' }}
        >
          {errorMessage}
        </Alert>
      </Snackbar>
    </>
  );
};

export default CopyButton;
