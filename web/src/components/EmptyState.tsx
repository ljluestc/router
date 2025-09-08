import React from 'react';
import {
  Box,
  Typography,
  Button,
  Paper,
  SvgIcon,
} from '@mui/material';
import {
  Inbox as InboxIcon,
  Search as SearchIcon,
  Add as AddIcon,
  Refresh as RefreshIcon,
  Error as ErrorIcon,
  Warning as WarningIcon,
  Info as InfoIcon,
} from '@mui/icons-material';

export type EmptyStateType = 'empty' | 'error' | 'warning' | 'info' | 'search' | 'loading';

interface EmptyStateProps {
  type?: EmptyStateType;
  title: string;
  description?: string;
  icon?: React.ReactNode;
  action?: {
    label: string;
    onClick: () => void;
    variant?: 'contained' | 'outlined' | 'text';
    color?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  };
  secondaryAction?: {
    label: string;
    onClick: () => void;
    variant?: 'contained' | 'outlined' | 'text';
    color?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  };
  size?: 'small' | 'medium' | 'large';
  showIcon?: boolean;
  elevation?: number;
}

const EmptyState: React.FC<EmptyStateProps> = ({
  type = 'empty',
  title,
  description,
  icon,
  action,
  secondaryAction,
  size = 'medium',
  showIcon = true,
  elevation = 1,
}) => {
  const getDefaultIcon = () => {
    switch (type) {
      case 'empty':
        return <InboxIcon />;
      case 'error':
        return <ErrorIcon />;
      case 'warning':
        return <WarningIcon />;
      case 'info':
        return <InfoIcon />;
      case 'search':
        return <SearchIcon />;
      case 'loading':
        return <RefreshIcon />;
      default:
        return <InboxIcon />;
    }
  };

  const getIconColor = () => {
    switch (type) {
      case 'error':
        return 'error.main';
      case 'warning':
        return 'warning.main';
      case 'info':
        return 'info.main';
      case 'search':
        return 'text.secondary';
      case 'loading':
        return 'primary.main';
      default:
        return 'text.secondary';
    }
  };

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          iconSize: 48,
          titleVariant: 'h6' as const,
          descriptionVariant: 'body2' as const,
          padding: 3,
        };
      case 'large':
        return {
          iconSize: 96,
          titleVariant: 'h4' as const,
          descriptionVariant: 'h6' as const,
          padding: 6,
        };
      default: // medium
        return {
          iconSize: 64,
          titleVariant: 'h5' as const,
          descriptionVariant: 'body1' as const,
          padding: 4,
        };
    }
  };

  const sizeProps = getSizeProps();

  return (
    <Paper
      elevation={elevation}
      sx={{
        p: sizeProps.padding,
        textAlign: 'center',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        minHeight: '200px',
      }}
    >
      {/* Icon */}
      {showIcon && (
        <Box
          sx={{
            color: getIconColor(),
            mb: 2,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
          }}
        >
          {icon ? (
            <SvgIcon sx={{ fontSize: sizeProps.iconSize }}>
              {icon}
            </SvgIcon>
          ) : (
            <SvgIcon sx={{ fontSize: sizeProps.iconSize }}>
              {getDefaultIcon()}
            </SvgIcon>
          )}
        </Box>
      )}

      {/* Title */}
      <Typography
        variant={sizeProps.titleVariant}
        component="h3"
        gutterBottom
        sx={{
          fontWeight: 'medium',
          color: 'text.primary',
        }}
      >
        {title}
      </Typography>

      {/* Description */}
      {description && (
        <Typography
          variant={sizeProps.descriptionVariant}
          color="text.secondary"
          paragraph
          sx={{
            maxWidth: '400px',
            mx: 'auto',
          }}
        >
          {description}
        </Typography>
      )}

      {/* Actions */}
      {(action || secondaryAction) && (
        <Box display="flex" gap={2} mt={3} flexWrap="wrap" justifyContent="center">
          {action && (
            <Button
              variant={action.variant || 'contained'}
              color={action.color || 'primary'}
              onClick={action.onClick}
              size="large"
            >
              {action.label}
            </Button>
          )}
          {secondaryAction && (
            <Button
              variant={secondaryAction.variant || 'outlined'}
              color={secondaryAction.color || 'primary'}
              onClick={secondaryAction.onClick}
              size="large"
            >
              {secondaryAction.label}
            </Button>
          )}
        </Box>
      )}
    </Paper>
  );
};

export default EmptyState;
