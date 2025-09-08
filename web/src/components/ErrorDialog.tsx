import React from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Typography,
  Box,
  IconButton,
  Alert,
  AlertTitle,
} from '@mui/material';
import {
  Close as CloseIcon,
  Error as ErrorIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';

interface ErrorDialogProps {
  open: boolean;
  onClose: () => void;
  onRetry?: () => void;
  title?: string;
  message: string;
  details?: string;
  showDetails?: boolean;
  showRetry?: boolean;
  retryText?: string;
  closeText?: string;
  maxWidth?: 'xs' | 'sm' | 'md' | 'lg' | 'xl';
  fullWidth?: boolean;
}

const ErrorDialog: React.FC<ErrorDialogProps> = ({
  open,
  onClose,
  onRetry,
  title = 'Error',
  message,
  details,
  showDetails = false,
  showRetry = false,
  retryText = 'Retry',
  closeText = 'Close',
  maxWidth = 'sm',
  fullWidth = true,
}) => {
  return (
    <Dialog
      open={open}
      onClose={onClose}
      maxWidth={maxWidth}
      fullWidth={fullWidth}
      PaperProps={{
        sx: {
          borderRadius: 2,
        },
      }}
    >
      <DialogTitle>
        <Box display="flex" alignItems="center" gap={1}>
          <ErrorIcon color="error" />
          <Typography variant="h6" component="span">
            {title}
          </Typography>
        </Box>
        <IconButton
          onClick={onClose}
          sx={{
            position: 'absolute',
            right: 8,
            top: 8,
          }}
        >
          <CloseIcon />
        </IconButton>
      </DialogTitle>
      <DialogContent>
        <Alert severity="error" sx={{ mb: 2 }}>
          <AlertTitle>Error</AlertTitle>
          {message}
        </Alert>
        
        {details && showDetails && (
          <Box
            sx={{
              mt: 2,
              p: 2,
              bgcolor: 'background.paper',
              borderRadius: 1,
              border: '1px solid',
              borderColor: 'divider',
            }}
          >
            <Typography
              variant="body2"
              component="pre"
              sx={{
                whiteSpace: 'pre-wrap',
                wordBreak: 'break-word',
                fontSize: '0.875rem',
              }}
            >
              {details}
            </Typography>
          </Box>
        )}
      </DialogContent>
      <DialogActions>
        {showRetry && onRetry && (
          <Button
            onClick={onRetry}
            variant="outlined"
            startIcon={<RefreshIcon />}
          >
            {retryText}
          </Button>
        )}
        <Button onClick={onClose} variant="contained">
          {closeText}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default ErrorDialog;
