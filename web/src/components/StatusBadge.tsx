import React from 'react';
import {
  Box,
  Typography,
  Chip,
  Avatar,
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
  Schedule as ScheduleIcon,
  Cancel as CancelIcon,
} from '@mui/icons-material';

export type StatusType = 
  | 'success' 
  | 'error' 
  | 'warning' 
  | 'info' 
  | 'pending' 
  | 'running' 
  | 'stopped' 
  | 'paused' 
  | 'scheduled' 
  | 'cancelled';

interface StatusBadgeProps {
  status: StatusType;
  label?: string;
  showIcon?: boolean;
  size?: 'small' | 'medium' | 'large';
  variant?: 'chip' | 'avatar' | 'text' | 'dot';
  tooltip?: string;
  color?: 'default' | 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
}

const StatusBadge: React.FC<StatusBadgeProps> = ({
  status,
  label,
  showIcon = true,
  size = 'medium',
  variant = 'chip',
  tooltip,
  color,
}) => {
  const getStatusConfig = () => {
    switch (status) {
      case 'success':
        return {
          icon: <CheckCircleIcon />,
          text: 'Success',
          color: 'success' as const,
        };
      case 'error':
        return {
          icon: <ErrorIcon />,
          text: 'Error',
          color: 'error' as const,
        };
      case 'warning':
        return {
          icon: <WarningIcon />,
          text: 'Warning',
          color: 'warning' as const,
        };
      case 'info':
        return {
          icon: <InfoIcon />,
          text: 'Info',
          color: 'info' as const,
        };
      case 'pending':
        return {
          icon: <ScheduleIcon />,
          text: 'Pending',
          color: 'default' as const,
        };
      case 'running':
        return {
          icon: <PlayIcon />,
          text: 'Running',
          color: 'primary' as const,
        };
      case 'stopped':
        return {
          icon: <StopIcon />,
          text: 'Stopped',
          color: 'default' as const,
        };
      case 'paused':
        return {
          icon: <PauseIcon />,
          text: 'Paused',
          color: 'warning' as const,
        };
      case 'scheduled':
        return {
          icon: <ScheduleIcon />,
          text: 'Scheduled',
          color: 'info' as const,
        };
      case 'cancelled':
        return {
          icon: <CancelIcon />,
          text: 'Cancelled',
          color: 'error' as const,
        };
      default:
        return {
          icon: <InfoIcon />,
          text: 'Unknown',
          color: 'default' as const,
        };
    }
  };

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          chipSize: 'small' as const,
          avatarSize: 20,
          fontSize: '0.75rem',
          dotSize: 8,
        };
      case 'large':
        return {
          chipSize: 'medium' as const,
          avatarSize: 32,
          fontSize: '1.1rem',
          dotSize: 12,
        };
      default: // medium
        return {
          chipSize: 'medium' as const,
          avatarSize: 24,
          fontSize: '0.875rem',
          dotSize: 10,
        };
    }
  };

  const config = getStatusConfig();
  const sizeProps = getSizeProps();
  const displayLabel = label || config.text;
  const statusColor = color || config.color;

  const renderContent = () => {
    switch (variant) {
      case 'text':
        return (
          <Box display="flex" alignItems="center" gap={0.5}>
            {showIcon && (
              <Box
                sx={{
                  color: `${statusColor}.main`,
                  display: 'flex',
                  alignItems: 'center',
                }}
              >
                {React.cloneElement(config.icon, {
                  sx: { fontSize: sizeProps.avatarSize },
                })}
              </Box>
            )}
            <Typography
              variant="body2"
              color={`${statusColor}.main`}
              sx={{ fontSize: sizeProps.fontSize }}
            >
              {displayLabel}
            </Typography>
          </Box>
        );

      case 'avatar':
        return (
          <Avatar
            sx={{
              width: sizeProps.avatarSize,
              height: sizeProps.avatarSize,
              bgcolor: `${statusColor}.main`,
              color: 'white',
            }}
          >
            {showIcon && (
              <Box
                sx={{
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                }}
              >
                {React.cloneElement(config.icon, {
                  sx: { fontSize: sizeProps.avatarSize * 0.6 },
                })}
              </Box>
            )}
          </Avatar>
        );

      case 'dot':
        return (
          <Box display="flex" alignItems="center" gap={1}>
            <Box
              sx={{
                width: sizeProps.dotSize,
                height: sizeProps.dotSize,
                borderRadius: '50%',
                bgcolor: `${statusColor}.main`,
                display: 'inline-block',
              }}
            />
            <Typography
              variant="body2"
              color={`${statusColor}.main`}
              sx={{ fontSize: sizeProps.fontSize }}
            >
              {displayLabel}
            </Typography>
          </Box>
        );

      default: // chip
        return (
          <Chip
            icon={showIcon ? config.icon : undefined}
            label={displayLabel}
            color={statusColor}
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

export default StatusBadge;
