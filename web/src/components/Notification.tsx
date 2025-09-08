import React from 'react';
import {
  Snackbar,
  Alert,
  AlertTitle,
  IconButton,
  Box,
  Typography,
} from '@mui/material';
import {
  Close as CloseIcon,
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  Warning as WarningIcon,
  Info as InfoIcon,
} from '@mui/icons-material';

interface NotificationProps {
  open: boolean;
  onClose: () => void;
  message: string;
  title?: string;
  severity?: 'success' | 'error' | 'warning' | 'info';
  duration?: number;
  action?: React.ReactNode;
  variant?: 'filled' | 'outlined' | 'standard';
}

const Notification: React.FC<NotificationProps> = ({
  open,
  onClose,
  message,
  title,
  severity = 'info',
  duration = 6000,
  action,
  variant = 'standard',
}) => {
  const getIcon = () => {
    switch (severity) {
      case 'success':
        return <CheckCircleIcon />;
      case 'error':
        return <ErrorIcon />;
      case 'warning':
        return <WarningIcon />;
      case 'info':
        return <InfoIcon />;
      default:
        return <InfoIcon />;
    }
  };

  return (
    <Snackbar
      open={open}
      autoHideDuration={duration}
      onClose={onClose}
      anchorOrigin={{
        vertical: 'top',
        horizontal: 'right',
      }}
      sx={{
        '& .MuiSnackbarContent-root': {
          minWidth: '300px',
        },
      }}
    >
      <Alert
        onClose={onClose}
        severity={severity}
        variant={variant}
        icon={getIcon()}
        action={
          <Box display="flex" alignItems="center" gap={1}>
            {action}
            <IconButton
              size="small"
              onClick={onClose}
              color="inherit"
            >
              <CloseIcon fontSize="small" />
            </IconButton>
          </Box>
        }
        sx={{
          width: '100%',
          '& .MuiAlert-message': {
            width: '100%',
          },
        }}
      >
        {title && (
          <AlertTitle>
            {title}
          </AlertTitle>
        )}
        <Typography variant="body2">
          {message}
        </Typography>
      </Alert>
    </Snackbar>
  );
};

export default Notification;
