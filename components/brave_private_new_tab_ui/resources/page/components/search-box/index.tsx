// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'
import styled from 'styled-components'
import { getLocale } from '$web-common/locale'

const Box = styled.div`
  display: flex;
  justify-content: center;
  flex-direction: column;
  align-items: center;
  gap: 40px;
  width: 100%;
`

const Form = styled.form`
  --bg-color: rgba(255, 255, 255, 0.22);
  --box-shadow: 0px 2px 70px rgba(0, 0, 0, 0.3);

  display: grid;
  grid-template-columns: 1fr 50px;
  align-items: center;
  width: 100%;
  height: 52px;
  font-family: ${p => p.theme.fontFamily.heading};
  color: white;
  font-size: 14px;
  font-weight: 400;
  background: var(--bg-color);
  border-radius: 8px;
  transition: box-shadow 0.3s ease-in-out;
  overflow: hidden;

  &:focus-within,
  &:hover {
    box-shadow: var(--box-shadow);
  }

  input[type="text"] {
    width: 100%;
    height: 36px;
    border: 0;
    background-color: transparent;
    padding: 5px 16px;

    &:focus {
      outline: 0;
    }

    &::placeholder {
      color: rgba(255,255,255,0.7);
    }
  }
`

const IconButton = styled.button`
  background: transparent;
  padding: 0;
  margin: 0;
  border: 0;
  width: 100%;
  height: 100%;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;

  &:hover {
    background: linear-gradient(304.74deg, #6F4CD2 15.81%, #BF14A2 63.17%, #F73A1C 100%);

    path {
      fill: white;
    }
  }
`

function BraveSearchLogo () {
    return (
        <div
            style={{
                display: 'flex',
                flexDirection: 'row',
                gap: '28px',
                alignItems: 'center'
            }}
        >
            <svg
                xmlns="http://www.w3.org/2000/svg"
                viewBox="0 0 64 64"
                width="72"
                height="72"
            >
                <path
                d="M13.75 0 H50.25 A13.75 13.75 0 0 1 64 13.75 V50.25 A13.75 13.75 0 0 1 50.25 64 H13.75 A13.75 13.75 0 0 1 0 50.25 V13.75 A13.75 13.75 0 0 1 13.75 0 Z"
                fill="#000"
                />
                <path d="M15 21.25H21.25V30L29 21.25H36.125L25.5 34L36.125 44.75H29L21.25 35.375V44.75H15Z" fill="#fff"/>
                <path d="M39.25 34H41.75V44.75H39.25ZM39.25 30.25H41.75V32H39.25Z" fill="#fff"/>
                <path d="M43.75 34H46.25V44.75H43.75Z" fill="#fff"/>
                <path d="M48.75 34H56.125V44.75H48.75Z" fill="#fff"/>
            </svg>

            <h1
                style={{
                margin: 0,
                fontSize: '42px',
                fontWeight: 600,
                letterSpacing: '0.18em',
                textTransform: 'uppercase',
                fontFamily: `
                    Inter,
                    IBM Plex Sans,
                    DIN Alternate,
                    DIN,
                    system-ui,
                    -apple-system,
                    BlinkMacSystemFont,
                    'Segoe UI',
                    sans-serif
                `
                }}
            >
                KILO
            </h1>
        </div>

    )
}

interface Props {
  onSubmit?: (value: string, openNewTab: boolean) => unknown
}

function Search (props: Props) {
  const [value, setValue] = React.useState('')
  const inputRef = React.useRef<HTMLInputElement>(null)

  const onInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setValue(e.currentTarget.value)
  }

  const handleFormBoxClick = () => {
    inputRef.current && inputRef.current.focus()
  }

  const handleSubmit = (e: React.ChangeEvent<HTMLFormElement>) => {
    e.preventDefault()
    props.onSubmit?.(value, false)
  }

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (value === '') return

    if ((e.metaKey || e.ctrlKey) && (e.key === 'Enter')) {
      props.onSubmit?.(value, true)
    }
  }

  return (
    <Box>
      <BraveSearchLogo />
      <Form onSubmit={handleSubmit} onClick={handleFormBoxClick} role="search" aria-label="Brave">
        <input ref={inputRef} onChange={onInputChange} onKeyDown={handleKeyDown} type="text" placeholder={getLocale('searchPlaceholderLabel')} value={value} autoCapitalize="off" autoComplete="off" autoCorrect="off" spellCheck="false" aria-label="Search" title="Search" aria-autocomplete="none" aria-haspopup="false" maxLength={2048} autoFocus />
        <IconButton data-test-id="submit_button" aria-label="Submit">
          <svg width="20" height="20" fill="none" xmlns="http://www.w3.org/2000/svg"><path fillRule="evenodd" clipRule="evenodd" d="M8 16a8 8 0 1 1 5.965-2.67l5.775 5.28a.8.8 0 1 1-1.08 1.18l-5.88-5.375A7.965 7.965 0 0 1 8 16Zm4.374-3.328a.802.802 0 0 0-.201.18 6.4 6.4 0 1 1 .202-.181Z" fill="url(#search_icon_gr)"/><defs><linearGradient id="search_icon_gr" x1="20" y1="20" x2="-2.294" y2="3.834" gradientUnits="userSpaceOnUse"><stop stopColor="#BF14A2"/><stop offset="1" stopColor="#F73A1C"/></linearGradient></defs></svg>
        </IconButton>
      </Form>
    </Box>
  )
}

export default Search
