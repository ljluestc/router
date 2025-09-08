import React from 'react';
import {
  Box,
  Typography,
  Chip,
  LinearProgress,
  Tooltip,
} from '@mui/material';
import {
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  Warning as WarningIcon,
  Info as InfoIcon,
  Pause as PauseIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
} from '@mui/icons-material';

export type StatusType = 
  | 'success' 
  | 'error' 
  | 'warning' 
  | 'info' 
  | 'pending' 
  | 'running' 
  | 'stopped' 
  | 'paused';

interface StatusIndicatorProps {
  status: StatusType;
  label?: string;
  showIcon?: boolean;
  showProgress?: boolean;
  progress?: number;
  tooltip?: string;
  size?: 'small' | 'medium' | 'large';
  variant?: 'chip' | 'text' | 'icon' | 'progress';
}

const StatusIndicator: React.FC<StatusIndicatorProps> = ({
  status,
  label,
  showIcon = true,
  showProgress = false,
  progress = 0,
  tooltip,
  size = 'medium',
  variant = 'chip',
}) => {
  const getStatusConfig = () => {
    switch (status) {
      case 'success':
        return {
          color: 'success' as const,
          icon: <CheckCircleIcon />,
          text: 'Success',
        };
      case 'error':
        return {
          color: 'error' as const,
          icon: <ErrorIcon />,
          text: 'Error',
        };
      case 'warning':
        return {
          color: 'warning' as const,
          icon: <WarningIcon />,
          text: 'Warning',
        };
      case 'info':
        return {
          color: 'info' as const,
          icon: <InfoIcon />,
          text: 'Info',
        };
      case 'pending':
        return {
          color: 'default' as const,
          icon: <PauseIcon />,
          text: 'Pending',
        };
      case 'running':
        return {
          color: 'primary' as const,
          icon: <PlayIcon />,
          text: 'Running',
        };
      case 'stopped':
        return {
          color: 'default' as const,
          icon: <StopIcon />,
          text: 'Stopped',
        };
      case 'paused':
        return {
          color: 'warning' as const,
          icon: <PauseIcon />,
          text: 'Paused',
        };
      default:
        return {
          color: 'default' as const,
          icon: <InfoIcon />,
          text: 'Unknown',
        };
    }
  };

  const config = getStatusConfig();
  const displayLabel = label || config.text;

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          iconSize: 16,
          fontSize: '0.75rem',
          chipSize: 'small' as const,
        };
      case 'large':
        return {
          iconSize: 24,
          fontSize: '1.1rem',
          chipSize: 'medium' as const,
        };
      default: // medium
        return {
          iconSize: 20,
          fontSize: '0.875rem',
          chipSize: 'medium' as const,
        };
    }
  };

  const sizeProps = getSizeProps();

  const renderContent = () => {
    switch (variant) {
      case 'text':
        return (
          <Typography
            variant="body2"
            color={`${config.color}.main`}
            sx={{ fontSize: sizeProps.fontSize }}
          >
            {displayLabel}
          </Typography>
        );

      case 'icon':
        return (
          <Box display="flex" alignItems="center" gap={1}>
            {showIcon && (
              <Box
                sx={{
                  color: `${config.color}.main`,
                  display: 'flex',
                  alignItems: 'center',
                }}
              >
                {React.cloneElement(config.icon, {
                  sx: { fontSize: sizeProps.iconSize },
                })}
              </Box>
            )}
            <Typography
              variant="body2"
              color={`${config.color}.main`}
              sx={{ fontSize: sizeProps.fontSize }}
            >
              {displayLabel}
            </Typography>
          </Box>
        );

      case 'progress':
        return (
          <Box width="100%">
            <Box display="flex" justifyContent="space-between" alignItems="center" mb={1}>
              <Typography
                variant="body2"
                color={`${config.color}.main`}
                sx={{ fontSize: sizeProps.fontSize }}
              >
                {displayLabel}
              </Typography>
              {showProgress && (
                <Typography
                  variant="body2"
                  color="text.secondary"
                  sx={{ fontSize: sizeProps.fontSize }}
                >
                  {Math.round(progress)}%
                </Typography>
              )}
            </Box>
            {showProgress && (
              <LinearProgress
                variant="determinate"
                value={progress}
                color={config.color}
                sx={{ height: 4, borderRadius: 2 }}
              />
            )}
          </Box>
        );

      default: // chip
        return (
          <Chip
            icon={showIcon ? config.icon : undefined}
            label={displayLabel}
            color={config.color}
            size={sizeProps.chipSize}
            variant="filled"
          />
        );
    }
  };

  const content = renderContent();

  if (tooltip) {
    return (
      <Tooltip title={tooltip} arrow>
        <Box display="inline-block">
          {content}
        </Box>
      </Tooltip>
    );
  }

  return content;
};

export default StatusIndicator;
