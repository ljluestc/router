import React from 'react';
import {
  Alert,
  AlertTitle,
  Box,
  Typography,
  Button,
  Collapse,
  IconButton,
} from '@mui/material';
import {
  ExpandMore as ExpandMoreIcon,
  ExpandLess as ExpandLessIcon,
  Refresh as RefreshIcon,
  BugReport as BugReportIcon,
} from '@mui/icons-material';

export type ErrorSeverity = 'error' | 'warning' | 'info';

interface ErrorMessageProps {
  title?: string;
  message: string;
  details?: string;
  severity?: ErrorSeverity;
  showDetails?: boolean;
  showRetry?: boolean;
  onRetry?: () => void;
  onDismiss?: () => void;
  dismissible?: boolean;
  collapsible?: boolean;
  showIcon?: boolean;
  variant?: 'filled' | 'outlined' | 'standard';
  size?: 'small' | 'medium' | 'large';
}

const ErrorMessage: React.FC<ErrorMessageProps> = ({
  title,
  message,
  details,
  severity = 'error',
  showDetails = false,
  showRetry = false,
  onRetry,
  onDismiss,
  dismissible = false,
  collapsible = false,
  showIcon = true,
  variant = 'standard',
  size = 'medium',
}) => {
  const [expanded, setExpanded] = React.useState(false);

  const handleToggleDetails = () => {
    setExpanded(!expanded);
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

  const renderDetails = () => {
    if (!details) return null;

    if (collapsible) {
      return (
        <Box mt={2}>
          <Button
            size="small"
            onClick={handleToggleDetails}
            startIcon={expanded ? <ExpandLessIcon /> : <ExpandMoreIcon />}
            sx={{ textTransform: 'none' }}
          >
            {expanded ? 'Hide Details' : 'Show Details'}
          </Button>
          <Collapse in={expanded} timeout="auto" unmountOnExit>
            <Box
              sx={{
                mt: 1,
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
          </Collapse>
        </Box>
      );
    }

    if (showDetails) {
      return (
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
      );
    }

    return null;
  };

  const renderActions = () => {
    if (!showRetry && !dismissible) return null;

    return (
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
        {dismissible && onDismiss && (
          <Button
            size="small"
            variant="text"
            onClick={onDismiss}
          >
            Dismiss
          </Button>
        )}
      </Box>
    );
  };

  return (
    <Alert
      severity={severity}
      variant={variant}
      icon={showIcon ? undefined : false}
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

      {renderDetails()}
      {renderActions()}
    </Alert>
  );
};

export default ErrorMessage;
