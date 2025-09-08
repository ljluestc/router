import React from 'react';
import {
  Alert,
  AlertTitle,
  Box,
  Typography,
  Button,
  IconButton,
} from '@mui/material';
import {
  Close as CloseIcon,
  CheckCircle as CheckCircleIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';

interface SuccessMessageProps {
  title?: string;
  message: string;
  details?: string;
  showDetails?: boolean;
  showRetry?: boolean;
  onRetry?: () => void;
  onDismiss?: () => void;
  dismissible?: boolean;
  showIcon?: boolean;
  variant?: 'filled' | 'outlined' | 'standard';
  size?: 'small' | 'medium' | 'large';
  autoHide?: boolean;
  autoHideDuration?: number;
}

const SuccessMessage: React.FC<SuccessMessageProps> = ({
  title,
  message,
  details,
  showDetails = false,
  showRetry = false,
  onRetry,
  onDismiss,
  dismissible = false,
  showIcon = true,
  variant = 'standard',
  size = 'medium',
  autoHide = false,
  autoHideDuration = 5000,
}) => {
  const [visible, setVisible] = React.useState(true);

  React.useEffect(() => {
    if (autoHide) {
      const timer = setTimeout(() => {
        setVisible(false);
        onDismiss?.();
      }, autoHideDuration);

      return () => clearTimeout(timer);
    }
  }, [autoHide, autoHideDuration, onDismiss]);

  const handleDismiss = () => {
    setVisible(false);
    onDismiss?.();
  };

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          fontSize: '0.75rem',
          padding: 1,
        };
      case 'large':
        return {
          fontSize: '1.1rem',
          padding: 3,
        };
      default: // medium
        return {
          fontSize: '0.875rem',
          padding: 2,
        };
    }
  };

  const sizeProps = getSizeProps();

  if (!visible) {
    return null;
  }

  return (
    <Alert
      severity="success"
      variant={variant}
      icon={showIcon ? undefined : false}
      action={
        dismissible ? (
          <IconButton
            size="small"
            onClick={handleDismiss}
            color="inherit"
          >
            <CloseIcon />
          </IconButton>
        ) : undefined
      }
      sx={{
        fontSize: sizeProps.fontSize,
        p: sizeProps.padding,
        '& .MuiAlert-message': {
          width: '100%',
        },
      }}
    >
      {title && (
        <AlertTitle sx={{ fontSize: sizeProps.fontSize }}>
          {title}
        </AlertTitle>
      )}
      
      <Typography
        variant="body2"
        sx={{ fontSize: sizeProps.fontSize }}
      >
        {message}
      </Typography>

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
              fontSize: sizeProps.fontSize,
            }}
          >
            {details}
          </Typography>
        </Box>
      )}

      {(showRetry || dismissible) && (
        <Box display="flex" gap={1} mt={2}>
          {showRetry && onRetry && (
            <Button
              size="small"
              variant="outlined"
              startIcon={<RefreshIcon />}
              onClick={onRetry}
            >
              Retry
            </Button>
          )}
          {dismissible && (
            <Button
              size="small"
              variant="text"
              onClick={handleDismiss}
            >
              Dismiss
            </Button>
          )}
        </Box>
      )}
    </Alert>
  );
};

export default SuccessMessage;
